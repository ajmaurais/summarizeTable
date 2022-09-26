
#include <iostream>
#include <fstream>

#include <params.hpp>
#include <tsvFile.hpp>

int main(int argc, char** argv)
{
    // Parse command line arguments
    params::Params args("Summarize information in tsv/csv files.");
    args.setSingleDashBehavior(params::Params::START_POSITIONAL);
    args.addOption('n', "", "Number of lines to look for data types. If reading from stdin, this option is ignored.",
                   params::Option::TYPE::INT);
    args.addOption('p', "rows", "Number of rows to print.", params::Option::TYPE::INT, "1");
    args.addOption("noHeader", "Don't treat first line as header.",
                   params::Option::BOOL, "false", params::Option::STORE_TRUE);
    args.addOption('F', "sep", "Field separator.", params::Option::CHAR, "\t");
    args.addOption('m', "mode", "Program output mode.", params::Option::STRING, "str", {"str", "summary"});
    args.addArgument("file", "File to look at. If no file is given, read from stdin.", 0, 1);
    args.parseArgs(argc, argv);

    // read data
    summarize::TsvFile tsvFile;
    tsvFile.setDelim(args.getOptionValue<char>("sep"));
    bool hasHeader = !args.getOptionValue<bool>("noHeader");
    if(args.getArgumentCount("file") == 0) {
        if(!tsvFile.read(std::cin, hasHeader)) {
            std::cerr << "Could not read table from stdin!\n";
            return 1;
        }
    } else {
        std::ifstream inF(args.getArgumentValue("file"));
        bool success = args.optionIsSet("n") ? tsvFile.read(inF, args.getOptionValue<int>("n"), hasHeader)
                                             : tsvFile.read(inF, hasHeader);
        if(!success) {
            std::cerr << "Could not read table from file!\n";
            return 1;
        }
    }

    // print summary data
    if(args.getOptionValue("mode") == "summary") {
        tsvFile.printSummary();
    } else {
        std::cout << (args.getArgumentCount("file") == 0 ? "stdin" : args.getArgumentValue("file")) << ": ";
        tsvFile.printStructure(args.getOptionValue<int>("rows"));
    }

    return 0;
}
