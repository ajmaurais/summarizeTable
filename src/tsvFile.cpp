//
// Created by Aaron Maurais on 9/24/22.
//

#include <cstdio>
#include <cctype>
#include <algorithm>

#include <tsvFile.hpp>

namespace {
    //! Upper bound on the number of bytes buffered for delimiter sniffing.
    const size_t SNIFF_MAX_BYTES = 1u << 16;   // 64 KiB
    //! Number of complete records to collect for the sniff sample when available.
    const size_t SNIFF_RECORDS = 20;

    //! Read a leading sample from \p is into \p sample. Quote-aware so that newlines
    //! embedded in quoted fields do not end a record early. Stops after SNIFF_RECORDS
    //! complete records, or once past SNIFF_MAX_BYTES with at least one complete record,
    //! or at EOF. Always reads at least the first complete record (up to EOF) so the
    //! sample is never truncated before its first line terminator.
    //! \return true if EOF was reached (so \p sample is the entire input).
    bool readSample(std::istream& is, std::string& sample) {
        std::streambuf* sb = is.rdbuf();
        bool inQuotes = false;
        size_t records = 0;
        while(true) {
            if(records >= SNIFF_RECORDS) return false;
            if(sample.size() >= SNIFF_MAX_BYTES && records >= 1) return false;
            int ci = sb->sbumpc();
            if(ci == EOF) return true;
            char c = static_cast<char>(ci);
            sample += c;
            if(c == '"') { inQuotes = !inQuotes; continue; }
            if(!inQuotes && (c == '\n' || c == '\r')) {
                if(c == '\r' && sb->sgetc() == '\n') { sb->sbumpc(); sample += '\n'; }
                records++;
            }
        }
    }

    //! A std::streambuf that yields the bytes of a prefix string first, then continues
    //! reading from an underlying streambuf. Lets a sniffed sample be re-read followed by
    //! the remainder of a non-seekable stream (e.g. stdin).
    class PrefixStreamBuf : public std::streambuf {
    public:
        PrefixStreamBuf(std::string prefix, std::streambuf* rest)
            : _prefix(std::move(prefix)), _pos(0), _rest(rest) {}
    protected:
        int_type underflow() override {   // peek, no advance
            if(_pos < _prefix.size()) return traits_type::to_int_type(_prefix[_pos]);
            return _rest ? _rest->sgetc() : traits_type::eof();
        }
        int_type uflow() override {       // read and advance
            if(_pos < _prefix.size()) return traits_type::to_int_type(_prefix[_pos++]);
            return _rest ? _rest->sbumpc() : traits_type::eof();
        }
    private:
        std::string _prefix;
        size_t _pos;
        std::streambuf* _rest;
    };
}

void summarize::TsvFile::_prepareInput(std::istream& is, std::string& sample) {
    sample.clear();
    bool complete = readSample(is, sample);
    stripUtf8Bom(sample);

    char sepDelim;
    size_t bytesToStrip;
    if(detectSepDirective(sample, sepDelim, bytesToStrip)) {
        sample.erase(0, bytesToStrip);     // never parse the directive line as data
        if(_sniff) _delim = sepDelim;      // "sep=" sets the delimiter unless one was explicit
    } else if(_sniff) {
        _delim = sniffDelimiter(sample, complete, _delim);
    }
    _sniff = false;
}

bool summarize::TsvFile::_read(std::istream& is, size_t nLines, bool allLines, bool hasHeader) {
    std::string sample;
    _prepareInput(is, sample);
    PrefixStreamBuf inBuf(std::move(sample), is.rdbuf());
    std::istream in(&inBuf);

    // Retain only the header (if any) plus the first _previewRows data rows. Every other
    // record is parsed into a reused scratch buffer purely to count it and measure the
    // widest record, so memory stays O(_previewRows * columns) regardless of file size.
    const size_t retainRecords = (hasHeader ? 1 : 0) + _previewRows;
    std::vector<std::vector<std::string> > retained;
    std::vector<std::string> scratch;
    size_t largestRow = 0;
    CsvParser parser(in, _delim);
    for(size_t i = 0;; i++) {
        if(!allLines && i >= nLines) break;
        bool keep = retained.size() < retainRecords;
        if(keep) retained.emplace_back();
        std::vector<std::string>& record = keep ? retained.back() : scratch;
        // Skip blank lines (records with no fields) anywhere in the input, matching the
        // behaviour of R's blank.lines.skip and pandas' skip_blank_lines.
        bool got;
        while((got = parser.nextRecord(record)) && record.empty()) {}
        if (!got) {
            if(keep) retained.pop_back();
            if(i == 0) {
                std::cerr << "ERROR: no data in input!" << std::endl;
                return false;
            }
            if(i > 1) {
                if(!allLines) std::cerr << "WARN: Fewer rows than " << std::to_string(nLines) << std::endl;
            }
            break;
        }

        _nRows++;
        largestRow = std::max(largestRow, record.size());
    }

    if(hasHeader) {
        _nRows--;
        for(size_t i = 0; i < largestRow; i++) {
            if(retained[0].size() > i)
                _headers.push_back(retained[0].at(i));
            else _headers.push_back("NO_NAME_COLUMN_" + std::to_string(i));
        }
    } else {
        for(size_t i = 0; i < largestRow; i++)
            _headers.push_back("COLUMN_" + std::to_string(i));
    }

    // populate _headerMap
    for(size_t i = 0; i < _headers.size(); i++) {
        _headerMap[_headers[i]] = i;
    }

    // populate _data by transposing the retained data rows
    for (size_t col = 0; col < largestRow; col++) {
        _data.emplace_back();
        for(size_t row = (hasHeader ? 1 : 0); row < retained.size(); row++) {
            if (retained[row].size() > col)
                _data[col].push_back(retained[row][col]);
            else _data[col].emplace_back();
        }
    }

    return true;
}

bool summarize::TsvFile::read(std::istream& is, size_t nLines, bool hasHeader) {
    return _read(is, nLines, false, hasHeader);
}

bool summarize::TsvFile::read(std::istream& is, bool hasHeader) {
    return _read(is, 0, true, hasHeader);
}

void summarize::TsvFile::printSummary() const {
    throw std::runtime_error("Not implemented!");
}

void summarize::TsvFile::printStructure(size_t nRows) const {
    std::cout << _nRows << " obs. of " << getNCols() << " variables" << std::endl;
    size_t maxRowI = numDigits(_headers.size());
    size_t maxRowLen = maxLength(_headers);
    for(size_t i = 0; i < _headers.size(); i++) {
        size_t indent = maxRowI - numDigits(i + 1);
        std::cout << std::string(indent, ' ') << std::to_string(i + 1) << ") " << _headers[i]
                  << std::string(maxRowLen - _headers.at(i).size(), ' ') + ':';
        // TODO: add data type here!
        // _nRows is the full row count; only getNPreviewRows() rows are retained in _data.
        size_t printRows = std::min(nRows, getNPreviewRows());
        for (size_t row = 0; row < printRows; row++)
            std::cout << ' ' << _data.at(i).at(row);
        std::cout << " ...\n";
    }
}

size_t summarize::maxLength(std::vector<std::string> strings) {
    size_t ret = 0;
    for(const auto& s: strings) ret = std::max(ret, s.size());
    return ret;
}

char summarize::delimFromExtension(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');
    if(dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return '\t';
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if(ext == "csv") return ',';
    return '\t';
}

bool summarize::hasParquetExtension(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');
    if(dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return false;
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return ext == "parquet" || ext == "pq";
}

bool summarize::stripUtf8Bom(std::string& s) {
    if(s.size() >= 3 &&
       static_cast<unsigned char>(s[0]) == 0xEF &&
       static_cast<unsigned char>(s[1]) == 0xBB &&
       static_cast<unsigned char>(s[2]) == 0xBF) {
        s.erase(0, 3);
        return true;
    }
    return false;
}

bool summarize::detectSepDirective(const std::string& sample, char& delim, size_t& bytesToStrip) {
    size_t i = 0;
    bool quoted = false;
    if(i < sample.size() && sample[i] == '"') { quoted = true; i++; }

    const std::string tag = "sep=";
    if(sample.size() < i + tag.size() + 1) return false;
    for(size_t j = 0; j < tag.size(); j++)
        if(std::tolower(static_cast<unsigned char>(sample[i + j])) != tag[j]) return false;
    i += tag.size();

    char d = sample[i++];               // the delimiter character
    if(quoted) {
        if(i >= sample.size() || sample[i] != '"') return false;
        i++;
    }
    // The directive must occupy the whole first line.
    if(i < sample.size()) {
        if(sample[i] == '\r') { i++; if(i < sample.size() && sample[i] == '\n') i++; }
        else if(sample[i] == '\n') i++;
        else return false;
    }
    delim = d;
    bytesToStrip = i;
    return true;
}

char summarize::sniffDelimiter(const std::string& sample, bool sampleComplete, char fallback) {
    static const char candidates[] = {'\t', ',', ';', '|'};   // also the tie-break preference order
    const size_t N = sizeof(candidates);
    std::vector<std::vector<int> > recordCounts(N);
    int cur[N] = {0};
    bool inQuotes = false;
    bool recordHasChars = false;

    auto flushRecord = [&]() {
        if(recordHasChars)
            for(size_t k = 0; k < N; k++) recordCounts[k].push_back(cur[k]);
        for(size_t k = 0; k < N; k++) cur[k] = 0;
        recordHasChars = false;
    };

    for(size_t i = 0; i < sample.size(); i++) {
        char c = sample[i];
        if(c == '"') { inQuotes = !inQuotes; recordHasChars = true; continue; }
        if(inQuotes) { recordHasChars = true; continue; }      // delimiters inside quotes don't count
        if(c == '\n') { flushRecord(); continue; }
        if(c == '\r') { if(i + 1 < sample.size() && sample[i + 1] == '\n') i++; flushRecord(); continue; }
        recordHasChars = true;
        for(size_t k = 0; k < N; k++) if(c == candidates[k]) cur[k]++;
    }
    if(sampleComplete) flushRecord();   // the last record is complete only at EOF

    // Pick the most consistent delimiter. Among equally consistent candidates prefer the
    // one yielding more fields (higher modal count), then the preferred order above.
    char best = fallback;
    double bestConsistency = -1.0;
    int bestMode = 0;
    for(size_t k = 0; k < N; k++) {
        const std::vector<int>& counts = recordCounts[k];
        if(counts.empty()) continue;
        std::map<int, int> freq;
        for(int v : counts) if(v > 0) freq[v]++;
        if(freq.empty()) continue;      // this delimiter never appears
        int mode = 0, modeFreq = 0;
        for(const auto& p : freq) if(p.second > modeFreq) { modeFreq = p.second; mode = p.first; }
        double consistency = static_cast<double>(modeFreq) / counts.size();
        if(consistency > bestConsistency || (consistency == bestConsistency && mode > bestMode)) {
            bestConsistency = consistency;
            bestMode = mode;
            best = candidates[k];
        }
    }
    return bestConsistency >= 0.5 ? best : fallback;
}

/**
 \brief Read the next record from the stream into \p fields.

 Implements an RFC 4180 style parser. Fields may be quoted with double quotes,
 in which case the delimiter, embedded \n, \r or \r\n line endings, and doubled
 "" quotes are treated as literal field content. Unquoted \n, \r and \r\n end
 the record. \p fields is cleared before reading.

 \param fields vector to populate with the fields of the next record.
 \return true if a record was read, false at end of input.
 */
bool summarize::CsvParser::nextRecord(std::vector<std::string>& fields) {
    fields.clear();
    std::string field;
    bool recordHasContent = false;
    enum State { START_FIELD, UNQUOTED, QUOTED, QUOTE_IN_QUOTED } state = START_FIELD;

    while(true) {
        int ci = _get();
        if(ci == EOF) {
            // A field is in progress (or pending after a trailing delimiter) iff
            // we are past START_FIELD or the record already has content.
            if(state != START_FIELD || recordHasContent)
                fields.push_back(field);
            return recordHasContent;
        }
        char c = static_cast<char>(ci);

        // Inside a quoted field everything is literal except a double quote.
        if(state == QUOTED) {
            if(c == '"') state = QUOTE_IN_QUOTED;
            else if(c == '\r') {
                field += '\n';
                if(_peek() == '\n') _get();
            }
            else field += c;
            continue;
        }

        // Just saw a '"' while inside a quoted field.
        if(state == QUOTE_IN_QUOTED) {
            if(c == '"') { field += '"'; state = QUOTED; }       // doubled "" -> literal "
            else if(c == _delim) { fields.push_back(field); field.clear(); state = START_FIELD; }
            else if(c == '\n') { fields.push_back(field); return true; }
            else if(c == '\r') { if(_peek() == '\n') _get(); fields.push_back(field); return true; }
            else { field += c; state = UNQUOTED; }              // lenient: text after closing quote
            continue;
        }

        // Unquoted context: START_FIELD or UNQUOTED.
        if(c == '"' && state == START_FIELD) { recordHasContent = true; state = QUOTED; }
        else if(c == _delim) { recordHasContent = true; fields.push_back(field); field.clear(); state = START_FIELD; }
        else if(c == '\n') { if(recordHasContent) fields.push_back(field); return true; }
        else if(c == '\r') { if(_peek() == '\n') _get(); if(recordHasContent) fields.push_back(field); return true; }
        else { recordHasContent = true; field += c; state = UNQUOTED; }
    }
}

