//
// Created by Aaron Maurais on 9/24/22.
//

#include <cstdio>

#include <tsvFile.hpp>

bool summarize::TsvFile::_read(std::istream& is, size_t nLines, bool allLines, bool hasHeader) {
    std::vector<std::vector<std::string> > data;
    size_t largestRow = 0;
    CsvParser parser(is, _delim);
    for(size_t i = 0;; i++) {
        if(!allLines && i >= nLines) break;
        // Fill the record in place to avoid copying it out of the parser.
        data.emplace_back();
        if (!parser.nextRecord(data.back())) {
            data.pop_back();
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
        largestRow = std::max(largestRow, data.back().size());
    }

    if(hasHeader) {
        _nRows--;
        for(size_t i = 0; i < largestRow; i++) {
            if(data[0].size() > i)
                _headers.push_back(data[0].at(i));
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

    // populate _data by transposing data
    for (size_t col = 0; col < largestRow; col++) {
        _data.emplace_back();
        for(size_t row = hasHeader; row < data.size(); row++) {
            if (data[row].size() > col)
                _data[col].push_back(data[row][col]);
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
        size_t printRows = std::min(nRows, _nRows);
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

