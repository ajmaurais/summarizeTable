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

    //! Default delimiter inferred from a file extension: ',' for .csv, '\t' otherwise.
    char delimFromExtension(const std::string& path);
    //! True if \p path has a .parquet or .pq extension.
    bool hasParquetExtension(const std::string& path);
    //! Remove a leading UTF-8 byte order mark from \p s. \return true if one was removed.
    bool stripUtf8Bom(std::string& s);
    //! Detect an Excel "sep=<char>" directive at the start of \p sample (optionally
    //! quoted). On success sets \p delim to the directive's character and \p bytesToStrip
    //! to the number of leading bytes (including the line terminator) to drop.
    bool detectSepDirective(const std::string& sample, char& delim, size_t& bytesToStrip);
    //! Infer the field delimiter from \p sample by frequency analysis, counting only
    //! candidate delimiters outside of quoted fields. \p sampleComplete is true when
    //! \p sample is the entire input (so its last record is complete). Returns \p fallback
    //! when no delimiter appears consistently.
    char sniffDelimiter(const std::string& sample, bool sampleComplete, char fallback);

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
        //! Total number of data rows seen in the input (not the number retained in _data).
        size_t _nRows;
        //! Number of leading data rows to retain for the preview; the rest are only counted.
        size_t _previewRows;
        char _delim;
        //! When true, _read infers the delimiter from the content (using _delim as a fallback).
        bool _sniff;

        bool _read(std::istream&, size_t, bool, bool = true);
        //! Read a leading sample from \p is, strip a UTF-8 BOM and any Excel "sep="
        //! directive, and (when _sniff is set) determine _delim from the content.
        //! \p sample returns the leading bytes still to be parsed.
        void _prepareInput(std::istream& is, std::string& sample);
    public:
        explicit TsvFile(char delim = '\t') {
            _delim = delim;
            _sniff = false;
            _nRows = 0;
            _previewRows = 1;
        }

        //! Set the number of leading data rows to retain in memory for the preview.
        void setPreviewRows(size_t n) {
            _previewRows = n;
        }

        //! Set an explicit delimiter; disables content sniffing.
        void setDelim(char delim) {
            _delim = delim;
            _sniff = false;
        }
        //! Infer the delimiter from the content when reading, falling back to \p fallback.
        void sniffDelim(char fallback) {
            _delim = fallback;
            _sniff = true;
        }
        char getDelim() const {
            return _delim;
        }
        bool read(std::istream&, size_t, bool = true);
        bool read(std::istream&, bool = true);
        //! Read column names, row count and a preview of the first getNPreviewRows()
        //! rows from the parquet file at \p path. Defined in parquetFile.cpp and only
        //! linked when the project is built with ENABLE_PARQUET.
        bool readParquet(const std::string& path);

        void printSummary() const;
        void printStructure(size_t nRows = 1) const;
        size_t getNRows() const {
            return _nRows;
        }
        size_t getNCols() const {
            return _headers.size();
        }
        //! Number of data rows actually retained in memory for the preview.
        size_t getNPreviewRows() const {
            return _data.empty() ? 0 : _data.front().size();
        }
    };
}

#endif //SUMMARIZE_TSVFILE_HPP
