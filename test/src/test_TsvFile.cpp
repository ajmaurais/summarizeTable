//
// Created by Aaron Maurais on 9/25/22.
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <testing.hpp>
#include <tsvFile.hpp>

//! Parse all records from \p text using \p delim and join them as
//! "f0|f1|f2;f0|f1" so a whole table can be compared in a single EXPECT_EQUAL.
static std::string parseAll(const std::string& text, char delim) {
    std::istringstream ss(text);
    summarize::CsvParser parser(ss, delim);
    std::vector<std::string> record;
    std::string ret;
    bool firstRecord = true;
    while(parser.nextRecord(record)) {
        if(!firstRecord) ret += ';';
        firstRecord = false;
        for(size_t i = 0; i < record.size(); i++) {
            if(i) ret += '|';
            ret += record[i];
        }
    }
    return ret;
}

//! Number of records parsed from \p text using \p delim.
static size_t countRecords(const std::string& text, char delim) {
    std::istringstream ss(text);
    summarize::CsvParser parser(ss, delim);
    std::vector<std::string> record;
    size_t n = 0;
    while(parser.nextRecord(record)) n++;
    return n;
}

START_TEST("tsvFile.hpp")
    START_SECTION("Stand alone functions")
        EXPECT_EQUAL(summarize::numDigits(0), 1)
        EXPECT_EQUAL(summarize::numDigits(9), 1)
        EXPECT_EQUAL(summarize::numDigits(10), 2)
        EXPECT_EQUAL(summarize::numDigits(11), 2)
        EXPECT_EQUAL(summarize::numDigits(100), 3)
    END_SECTION

    START_SECTION("CsvParser basic splitting")
        EXPECT_EQUAL(parseAll("a\tb\tc\n", '\t'), std::string("a|b|c"))
        EXPECT_EQUAL(parseAll("a,b,c\n", ','), std::string("a|b|c"))
        EXPECT_EQUAL(parseAll("a\tb\tc", '\t'), std::string("a|b|c"))           // no trailing newline
        EXPECT_EQUAL(parseAll("a\tb\nc\td\n", '\t'), std::string("a|b;c|d"))    // two records
        EXPECT_EQUAL(parseAll("a\t\tc\n", '\t'), std::string("a||c"))           // empty middle field
        EXPECT_EQUAL(parseAll("a\tb\t\n", '\t'), std::string("a|b|"))           // trailing empty field
    END_SECTION

    START_SECTION("CsvParser line endings")
        EXPECT_EQUAL(parseAll("a\tb\r\nc\td\r\n", '\t'), std::string("a|b;c|d"))    // CRLF
        EXPECT_EQUAL(parseAll("a\tb\rc\td\r", '\t'), std::string("a|b;c|d"))        // lone CR
        EXPECT_EQUAL(countRecords("a\n\nb\n", '\t'), static_cast<size_t>(3))        // blank line is a record
    END_SECTION

    START_SECTION("CsvParser quoted fields")
        EXPECT_EQUAL(parseAll("\"a,b\",c\n", ','), std::string("a,b|c"))            // delimiter inside quotes
        EXPECT_EQUAL(parseAll("\"a\"\"b\",c\n", ','), std::string("a\"b|c"))        // doubled "" -> "
        EXPECT_EQUAL(parseAll("\"a\nb\",c\n", ','), std::string("a\nb|c"))          // embedded newline
        EXPECT_EQUAL(countRecords("\"a\nb\",c\nd,e\n", ','), static_cast<size_t>(2))// embedded newline keeps one record
        EXPECT_EQUAL(parseAll("\"\",a\n", ','), std::string("|a"))                  // empty quoted field
        EXPECT_EQUAL(parseAll("\"a\r\nb\",c\n", ','), std::string("a\nb|c"))        // embedded CRLF normalized
    END_SECTION

    START_SECTION("CsvParser edge cases")
        EXPECT_EQUAL(countRecords("", '\t'), static_cast<size_t>(0))               // empty input
        EXPECT_EQUAL(countRecords("\n", '\t'), static_cast<size_t>(1))             // single blank line
        EXPECT_EQUAL(parseAll("single\n", '\t'), std::string("single"))            // one column
    END_SECTION
END_TEST
