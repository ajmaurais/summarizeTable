//
// Parquet support for summarize. This is the only translation unit that depends on
// Apache Arrow; it is compiled and linked only when the project is built with
// ENABLE_PARQUET so the core TSV/CSV tool has no Arrow dependency.
//

#include <algorithm>
#include <memory>

#include <arrow/api.h>
#include <arrow/io/file.h>
#include <parquet/arrow/reader.h>

#include <tsvFile.hpp>

bool summarize::TsvFile::readParquet(const std::string& path) {
    arrow::Result<std::shared_ptr<arrow::io::ReadableFile> > infile =
        arrow::io::ReadableFile::Open(path);
    if(!infile.ok()) {
        std::cerr << "ERROR: " << infile.status().ToString() << std::endl;
        return false;
    }

    // Cap the Arrow batch size so only the preview rows are ever materialized.
    parquet::ArrowReaderProperties props;
    props.set_batch_size(_previewRows < 1 ? 1 : static_cast<int64_t>(_previewRows));

    parquet::arrow::FileReaderBuilder builder;
    arrow::Status status = builder.Open(*infile);
    if(!status.ok()) {
        std::cerr << "ERROR: " << status.ToString() << std::endl;
        return false;
    }
    builder.properties(props);

    std::unique_ptr<parquet::arrow::FileReader> reader;
    status = builder.Build(&reader);
    if(!status.ok()) {
        std::cerr << "ERROR: " << status.ToString() << std::endl;
        return false;
    }

    // The row count comes straight from the file footer; no row scan is needed.
    _nRows = static_cast<size_t>(reader->parquet_reader()->metadata()->num_rows());

    // Column names from the schema.
    std::shared_ptr<arrow::Schema> schema;
    status = reader->GetSchema(&schema);
    if(!status.ok()) {
        std::cerr << "ERROR: " << status.ToString() << std::endl;
        return false;
    }
    for(int i = 0; i < schema->num_fields(); i++)
        _headers.push_back(schema->field(i)->name());
    for(size_t i = 0; i < _headers.size(); i++)
        _headerMap[_headers[i]] = i;

    // Read a single (capped) batch for the preview values.
    arrow::Result<std::shared_ptr<arrow::RecordBatchReader> > batchReader =
        reader->GetRecordBatchReader();
    if(!batchReader.ok()) {
        std::cerr << "ERROR: " << batchReader.status().ToString() << std::endl;
        return false;
    }
    std::shared_ptr<arrow::RecordBatch> batch;
    status = (*batchReader)->ReadNext(&batch);
    if(!status.ok()) {
        std::cerr << "ERROR: " << status.ToString() << std::endl;
        return false;
    }

    size_t previewN = batch ? std::min(_previewRows, static_cast<size_t>(batch->num_rows())) : 0;

    // Populate _data column-wise, stringifying each retained cell.
    for(int col = 0; col < schema->num_fields(); col++) {
        _data.emplace_back();
        if(!batch) continue;
        std::shared_ptr<arrow::Array> column = batch->column(col);
        for(size_t row = 0; row < previewN; row++) {
            arrow::Result<std::shared_ptr<arrow::Scalar> > scalar = column->GetScalar(row);
            if(scalar.ok()) _data[col].push_back((*scalar)->ToString());
            else _data[col].emplace_back();
        }
    }

    return true;
}
