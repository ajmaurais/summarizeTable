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

    START_SECTION("delimFromExtension")
        EXPECT_EQUAL(summarize::delimFromExtension("foo.csv"), ',')
        EXPECT_EQUAL(summarize::delimFromExtension("foo.CSV"), ',')                // case-insensitive
        EXPECT_EQUAL(summarize::delimFromExtension("foo.tsv"), '\t')
        EXPECT_EQUAL(summarize::delimFromExtension("foo.txt"), '\t')
        EXPECT_EQUAL(summarize::delimFromExtension("foo"), '\t')                   // no extension
        EXPECT_EQUAL(summarize::delimFromExtension("path/to/file.csv"), ',')
        EXPECT_EQUAL(summarize::delimFromExtension("my.dir/file"), '\t')           // dot is in a dir, not the file
    END_SECTION

    START_SECTION("stripUtf8Bom")
        std::string withBom = "\xEF\xBB\xBF" "hello";
        EXPECT_EQUAL(summarize::stripUtf8Bom(withBom), true)
        EXPECT_EQUAL(withBom, std::string("hello"))
        std::string noBom = "hello";
        EXPECT_EQUAL(summarize::stripUtf8Bom(noBom), false)
        EXPECT_EQUAL(noBom, std::string("hello"))
    END_SECTION

    START_SECTION("detectSepDirective")
        char d = '\0';
        size_t strip = 0;
        EXPECT_EQUAL(summarize::detectSepDirective("sep=,\nx,y\n", d, strip), true)
        EXPECT_EQUAL(d, ',')
        EXPECT_EQUAL(strip, static_cast<size_t>(6))                                // "sep=," + '\n'
        EXPECT_EQUAL(summarize::detectSepDirective("sep=;\r\nx;y\n", d, strip), true)
        EXPECT_EQUAL(d, ';')
        EXPECT_EQUAL(strip, static_cast<size_t>(7))                                // "sep=;" + '\r\n'
        EXPECT_EQUAL(summarize::detectSepDirective("\"sep=,\"\nx\n", d, strip), true)  // quoted form
        EXPECT_EQUAL(d, ',')
        EXPECT_EQUAL(summarize::detectSepDirective("name,age\n", d, strip), false) // ordinary header
        EXPECT_EQUAL(summarize::detectSepDirective("sep=,extra\n", d, strip), false) // not a lone directive
    END_SECTION

    START_SECTION("sniffDelimiter")
        EXPECT_EQUAL(summarize::sniffDelimiter("a\tb\tc\n1\t2\t3\n", true, '\t'), '\t')
        EXPECT_EQUAL(summarize::sniffDelimiter("a,b,c\n1,2,3\n", true, '\t'), ',')
        EXPECT_EQUAL(summarize::sniffDelimiter("a;b;c\n1;2;3\n", true, '\t'), ';')
        EXPECT_EQUAL(summarize::sniffDelimiter("a|b|c\n1|2|3\n", true, '\t'), '|')
        // A tab-delimited table whose last column always contains one comma -> tab wins.
        EXPECT_EQUAL(summarize::sniffDelimiter("name\tnote\nAlice\thi, there\nBob\tyo, x\n", true, '\t'), '\t')
        // Commas inside quoted fields must not be counted.
        EXPECT_EQUAL(summarize::sniffDelimiter("\"a,b\"\tc\n\"d,e\"\tf\n", true, '\t'), '\t')
        // No delimiter present -> fall back to the supplied default.
        EXPECT_EQUAL(summarize::sniffDelimiter("alpha\nbeta\n", true, '\t'), '\t')
        EXPECT_EQUAL(summarize::sniffDelimiter("alpha\nbeta\n", true, ','), ',')
    END_SECTION

    START_SECTION("TsvFile sniffing integration")
        {   // .csv content is sniffed as comma even with a tab fallback
            std::istringstream ss("a,b,c\n1,2,3\n");
            summarize::TsvFile f;
            f.sniffDelim('\t');
            EXPECT_EQUAL(f.read(ss, true), true)
            EXPECT_EQUAL(f.getDelim(), ',')
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(3))
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(1))
        }
        {   // "sep=" directive sets the delimiter and is not counted as a data row
            std::istringstream ss("sep=;\nx;y\n1;2\n");
            summarize::TsvFile f;
            f.sniffDelim('\t');
            f.read(ss, true);
            EXPECT_EQUAL(f.getDelim(), ';')
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(2))
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(1))
        }
        {   // explicit setDelim overrides content sniffing
            std::istringstream ss("a,b,c\n1,2,3\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.read(ss, true);
            EXPECT_EQUAL(f.getDelim(), '\t')
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(1))
        }
    END_SECTION

    START_SECTION("TsvFile streaming row count")
        {   // every row is counted but only _previewRows are retained in memory
            std::string text = "h1\th2\n";
            for(int i = 0; i < 100; i++) text += "a\tb\n";
            std::istringstream ss(text);
            summarize::TsvFile f;
            f.setDelim('\t');
            f.setPreviewRows(3);
            f.read(ss, true);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(100))         // full count
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(2))
            EXPECT_EQUAL(f.getNPreviewRows(), static_cast<size_t>(3))    // only 3 retained
        }
        {   // fewer data rows than the preview limit
            std::istringstream ss("h1\th2\nx\ty\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.setPreviewRows(5);
            f.read(ss, true);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(1))
            EXPECT_EQUAL(f.getNPreviewRows(), static_cast<size_t>(1))
        }
        {   // a wide row beyond the retained set still sets the column count
            std::istringstream ss("h1\th2\nx\ty\np\tq\tr\ts\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.setPreviewRows(1);
            f.read(ss, true);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(2))
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(4))           // widest row wins
            EXPECT_EQUAL(f.getNPreviewRows(), static_cast<size_t>(1))
        }
        {   // a capped read (-n) still limits the row count as before
            std::string text = "h1\th2\n";
            for(int i = 0; i < 100; i++) text += "a\tb\n";
            std::istringstream ss(text);
            summarize::TsvFile f;
            f.setDelim('\t');
            f.read(ss, static_cast<size_t>(10), true);                  // read 10 records incl. header
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(9))
        }
    END_SECTION

    START_SECTION("TsvFile skips blank lines")
        {   // a trailing blank line is not an observation
            std::istringstream ss("h1\th2\nx\ty\n\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.read(ss, true);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(1))
        }
        {   // a blank line in the middle is skipped
            std::istringstream ss("h1\th2\n\nx\ty\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.read(ss, true);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(1))
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(2))
        }
        {   // a leading blank line does not become the header
            std::istringstream ss("\nh1\th2\nx\ty\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.read(ss, true);
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(2))
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(1))
        }
        {   // a row of empty fields ("\t") is data, not a blank line
            std::istringstream ss("h1\th2\n\t\nx\ty\n");
            summarize::TsvFile f;
            f.setDelim('\t');
            f.read(ss, true);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(2))
        }
    END_SECTION
END_TEST
