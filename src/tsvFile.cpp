//
// Created by Aaron Maurais on 9/24/22.
//

#include <tsvFile.hpp>

bool summarize::TsvFile::_read(std::istream& is, size_t nLines, bool allLines, bool hasHeader) {
    std::string line;
    std::vector<std::vector<std::string> > data;
    size_t largestRow = 0;
    for(size_t i = 0;; i++) {
        if(!allLines && i >= nLines) break;
        if (!safeGetLine(is, line)) {
            if(i == 0) {
                std::cerr << "ERROR: no data in input!" << std::endl;
                return false;
            }
            if(i > 1) {
                if(!allLines) std::cerr << "WARN: Fewer rows than " << std::to_string(nLines) << std::endl;
            }
            break;
        }

        data.emplace_back();
        _nRows++;
        split(line, _delim, data.back());
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
        size_t indent = maxRowI - numDigits(i);
        std::cout << std::string(indent, ' ') << std::to_string(i) << ") " << _headers[i]
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
 \brief Get next line from \p is and store it in \p t.

 std::getline only handles \\n. safeGetLine handles \\n \\r \\r\\n.
 \param is stream to read from
 \param s string to store next line in \p t is cleared prior to adding new string.
 \return ref to \p is after reading next line.
 */
std::istream& summarize::safeGetLine(std::istream& is, std::string& s)
{
    s.clear();
    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    while(true){
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return is;
            case '\r':
                if(sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case -1:
                // Also handle the case when the last line has no line ending
                if(s.empty())
                    is.setstate(std::ios::eofbit);
                return is;
            default:
                s += (char)c;
        }
    }
}

//!split \p str by \p delim and populate each split into \p elems
void summarize::split (const std::string& str, const char delim, std::vector<std::string>& elems)
{
    elems.clear();
    elems.shrink_to_fit();
    std::stringstream ss (str);
    std::string item;

    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

