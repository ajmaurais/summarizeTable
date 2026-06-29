//
// Tests for parquet reading. Builds a small parquet file with Arrow, then reads it
// back through summarize::TsvFile::readParquet.
//

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include <arrow/api.h>
#include <arrow/io/file.h>
#include <parquet/arrow/writer.h>

#include <testing.hpp>
#include <tsvFile.hpp>

//! Write a 3-row, 2-column (int64 "id", string "name") parquet file to \p path.
static bool writeFixture(const std::string& path) {
    arrow::Int64Builder idBuilder;
    arrow::StringBuilder nameBuilder;
    if(!idBuilder.AppendValues({10, 20, 30}).ok()) return false;
    if(!nameBuilder.AppendValues({"alice", "bob", "carol"}).ok()) return false;

    std::shared_ptr<arrow::Array> idArray, nameArray;
    if(!idBuilder.Finish(&idArray).ok()) return false;
    if(!nameBuilder.Finish(&nameArray).ok()) return false;

    std::shared_ptr<arrow::Schema> schema = arrow::schema(
        {arrow::field("id", arrow::int64()), arrow::field("name", arrow::utf8())});
    std::shared_ptr<arrow::Table> table = arrow::Table::Make(schema, {idArray, nameArray});

    arrow::Result<std::shared_ptr<arrow::io::FileOutputStream> > out =
        arrow::io::FileOutputStream::Open(path);
    if(!out.ok()) return false;
    return parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), *out, 1024).ok();
}

//! Capture the output of printStructure so preview values can be asserted on.
static std::string captureStructure(const summarize::TsvFile& f, size_t nRows) {
    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    f.printStructure(nRows);
    std::cout.rdbuf(old);
    return oss.str();
}

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

START_TEST("parquetFile")
    const std::string path = "test_parquet_fixture.parquet";
    EXPECT_EQUAL(writeFixture(path), true)

    START_SECTION("read structure and row count")
        {
            summarize::TsvFile f;
            f.setPreviewRows(2);
            EXPECT_EQUAL(f.readParquet(path), true)
            EXPECT_EQUAL(f.getNCols(), static_cast<size_t>(2))
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(3))          // from footer metadata
            EXPECT_EQUAL(f.getNPreviewRows(), static_cast<size_t>(2))   // capped to preview rows
        }
    END_SECTION

    START_SECTION("preview values are stringified")
        {
            summarize::TsvFile f;
            f.setPreviewRows(2);
            f.readParquet(path);
            std::string out = captureStructure(f, 2);
            EXPECT_EQUAL(contains(out, "3 obs. of 2 variables"), true)
            EXPECT_EQUAL(contains(out, "id"), true)
            EXPECT_EQUAL(contains(out, "name"), true)
            EXPECT_EQUAL(contains(out, "10"), true)                     // int64 cell -> "10"
            EXPECT_EQUAL(contains(out, "alice"), true)                  // string cell
        }
    END_SECTION

    START_SECTION("preview retains fewer rows than the table")
        {
            summarize::TsvFile f;                                       // default previewRows == 1
            f.readParquet(path);
            EXPECT_EQUAL(f.getNRows(), static_cast<size_t>(3))
            EXPECT_EQUAL(f.getNPreviewRows(), static_cast<size_t>(1))
        }
    END_SECTION
END_TEST
