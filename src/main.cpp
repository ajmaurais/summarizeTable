
#include <iostream>

#include <params.hpp>

int main(int argc, char** argv)
{
    // Parse command line arguments
    params::Params args("Summarize information in tsv/csv files.");
    args.setSingleDashBehavior(params::Params::START_POSITIONAL);
    args.addOption("n", "", "Number of lines to look for data types.", params::Option::TYPE::INT, "5");
    args.addOption("p", "rows", "Number of rows to print.", params::Option::TYPE::INT, "1");
    args.addOption("F", "sep", "Field separator.", params::Option::STRING, "\t");
    args.addArgument("file", "File to look at. If no file is given, read from stdin.");
    args.parseArgs(argc, argv);

    return 0;
}
