
#include <iostream>
#include <fstream>

#include <argparse.hpp>
#include <tsvFile.hpp>

int main(int argc, char** argv)
{
    // Parse command line arguments
    argparse::ArgumentParser args("Summarize information in tsv/csv files.");
    args.setSingleDashBehavior(argparse::ArgumentParser::START_POSITIONAL);
    args.addOption<int>('n', "", "Number of lines to look for data types. If reading from stdin, this option is ignored.");
    args.addOption<int>('p', "rows", "Number of rows to print.", 1);
    args.addOption<bool>("noHeader", "Don't treat first line as header.", false, argparse::Option::STORE_TRUE);
    args.addOption<char>('F', "sep", "Field separator.", '\t');
    args.addOption<std::string>('m', "mode", "Program output mode.", "str", {"str", "summary"});
    args.addArgument("file", "File to look at. If no file is given, read from stdin.", 0, 1);
    if(!args.parseArgs(argc, argv))
        return 1;

    // read data
    summarize::TsvFile tsvFile;
    bool fileGiven = args.getArgument("file").getArgCount() > 0;
    std::string filePath = fileGiven ? args.getArgumentValue("file") : "";

    // Only the rows that will be printed need to be held in memory; the rest of the
    // file is streamed through just to count it.
    int previewRows = args.getOptionValue<int>("rows");
    tsvFile.setPreviewRows(previewRows < 0 ? 0 : static_cast<size_t>(previewRows));

    if(fileGiven && summarize::hasParquetExtension(filePath)) {
        // Parquet is columnar and self-describing, so the delimiter / header options
        // do not apply; read it directly through Arrow.
#ifdef ENABLE_PARQUET
        if(!tsvFile.readParquet(filePath)) {
            std::cerr << "Could not read parquet file!\n";
            return 1;
        }
#else
        std::cerr << "Parquet support was not enabled in this build.\n";
        return 1;
#endif
    } else {
        if(args.optionIsSet("sep")) {
            // Explicit separator always wins.
            tsvFile.setDelim(args.getOptionValue<char>("sep"));
        } else {
            // Otherwise infer the separator from the content, falling back to the file
            // extension (.csv -> ',') or a tab for stdin.
            char fallback = fileGiven ? summarize::delimFromExtension(filePath) : '\t';
            tsvFile.sniffDelim(fallback);
        }
        bool hasHeader = !args.getOptionValue<bool>("noHeader");
        if(!fileGiven) {
            if(!tsvFile.read(std::cin, hasHeader)) {
                std::cerr << "Could not read table from stdin!\n";
                return 1;
            }
        } else {
            std::ifstream inF(filePath);
            bool success = args.optionIsSet("n") ? tsvFile.read(inF, args.getOptionValue<int>("n"), hasHeader)
                                                 : tsvFile.read(inF, hasHeader);
            if(!success) {
                std::cerr << "Could not read table from file!\n";
                return 1;
            }
        }
    }

    // print summary data
    if(args.getOptionValue("mode") == "summary") {
        tsvFile.printSummary();
    } else {
        std::cout << (fileGiven ? filePath : "stdin") << ": ";
        tsvFile.printStructure(args.getOptionValue<int>("rows"));
    }

    return 0;
}
