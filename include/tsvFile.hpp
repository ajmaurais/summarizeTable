//
// Created by Aaron Maurais on 9/24/22.
//

#ifndef SUMMARIZE_TSVFILE_HPP
#define SUMMARIZE_TSVFILE_HPP

#include <iostream>
#include <vector>
#include <map>

namespace summarize {

    size_t maxLength(std::vector<std::string>);
    template <typename T> size_t numDigits(T unsignedInteger) {
        int digits = 0;
        do {
            unsignedInteger /= 10;
            digits++;
        } while (unsignedInteger);
        return digits;
    }

    //! Streaming RFC 4180 CSV/TSV record parser.
    //!
    //! Handles quoted fields, doubled "" quotes inside quoted fields, embedded
    //! newlines inside quoted fields (a single record may span multiple physical
    //! lines), and \n, \r and \r\n line endings. Embedded line endings inside a
    //! quoted field are normalized to \n.
    class CsvParser {
    private:
        std::streambuf* _sb;
        char _delim;

        int _get() { return _sb->sbumpc(); }
        int _peek() { return _sb->sgetc(); }
    public:
        CsvParser(std::istream& is, char delim) : _sb(is.rdbuf()), _delim(delim) {}

        //! Read the next record into \p fields (cleared first).
        //! \return true if a record was read, false at end of input.
        bool nextRecord(std::vector<std::string>& fields);
    };

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
