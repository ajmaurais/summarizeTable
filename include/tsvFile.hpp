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

    class TsvFile {
    public:
        enum TYPE {
            STRING, INT, BOOL, FLOAT, Last, First = STRING
        };
        std::string typeToString;
    private:
        std::vector<std::string> _headers;
        std::map<std::string, size_t> _headerMap;
        std::vector<std::vector<std::string> > _data;
        std::vector<TYPE> _dataTypes;
        //! Actual number of rows in _data
        size_t _nRows;
        char _delim;

        bool _read(std::istream&, size_t, bool, bool = true);
    public:
        explicit TsvFile(char delim = '\t') {
            _delim = delim;
            _nRows = 0;
        }

        void setDelim(char delim) {
            _delim = delim;
        }
        bool read(std::istream&, size_t, bool = true);
        bool read(std::istream&, bool = true);

        void printSummary() const;
        void printStructure(size_t nRows = 1) const;
        size_t getNRows() const {
            return _nRows;
        }
        size_t getNCols() const {
            return _headers.size();
        }
    };
}

#endif //SUMMARIZE_TSVFILE_HPP
