
#include <iostream>
#include <fstream>

#include <params.hpp>
#include <tsvFile.hpp>

int main(int argc, char** argv)
{
    // Parse command line arguments
    params::Params args("Summarize information in tsv/csv files.");
    args.setSingleDashBehavior(params::Params::START_POSITIONAL);
    args.addOption<int>('n', "", "Number of lines to look for data types. If reading from stdin, this option is ignored.");
    args.addOption<int>('p', "rows", "Number of rows to print.", 1);
    args.addOption<bool>("noHeader", "Don't treat first line as header.", false, params::Option::STORE_TRUE);
    args.addOption<char>('F', "sep", "Field separator.", '\t');
    args.addOption<std::string>('m', "mode", "Program output mode.", "str", {"str", "summary"});
    args.addArgument("file", "File to look at. If no file is given, read from stdin.", 0, 1);
    if(!args.parseArgs(argc, argv))
        return 1;

    // read data
    summarize::TsvFile tsvFile;
    tsvFile.setDelim(args.getOptionValue<char>("sep"));
    bool hasHeader = !args.getOptionValue<bool>("noHeader");
    if(args.getArgument("file").getArgCount() == 0) {
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
        std::cout << (args.getArgument("file").getArgCount() == 0 ? "stdin" : args.getArgumentValue("file")) << ": ";
        tsvFile.printStructure(args.getOptionValue<int>("rows"));
    }

    return 0;
}
