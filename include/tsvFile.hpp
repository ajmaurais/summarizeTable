//
// Created by Aaron Maurais on 9/24/22.
//

#ifndef SUMMARIZE_TSVFILE_HPP
#define SUMMARIZE_TSVFILE_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

namespace summarize {

    std::istream& safeGetLine(std::istream& is, std::string& s);
    void split(const std::string& s, char delim, std::vector<std::string>& elems);
    size_t maxLength(std::vector<std::string>);
    template <typename T> size_t numDigits(T unsignedInteger) {
        int digits = 0;
        do {
            unsignedInteger /= 10;
            digits++;
        } while (unsignedInteger);
        return digits;
    }

    class DataType;

    class TsvFile {
    private:
        std::vector<std::string> _headers;
        std::map<std::string, size_t> _headerMap;
        std::vector<std::vector<std::string> > _data;
        std::vector<std::string> _dataTypes;
        size_t _nLines;
        size_t _nCols;
        char _delim;
        bool _allLines;

    public:
        TsvFile() {
            _delim = '\t';
            _nLines = 0;
            _nCols = 0;
        }

        void setDelim(char delim) {
            _delim = delim;
        }
        bool read(std::istream&, size_t, bool = true);
        size_t getNRows() const {
            return _nLines;
        }
        size_t getNCols() const {
            return _nCols;
        }
    };
}

#endif //SUMMARIZE_TSVFILE_HPP
