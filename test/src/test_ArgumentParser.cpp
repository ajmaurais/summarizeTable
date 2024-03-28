//
// Created by Aaron Maurais on 9/21/22.
//

#include <iostream>

#include <testing.hpp>
#include <argparse.hpp>

int const TEST_ARGV_SIZE = 256;

int populateArgArray(const std::vector<std::string>& argVector, char* argArray[TEST_ARGV_SIZE]) {
    for(size_t i = 0; i < argVector.size(); i++) {
        argArray[i] = new char[argVector[i].size()];
        strcpy(argArray[i], argVector[i].c_str());
    }
    return argVector.size();
}

START_TEST("ArgumentParser")
    START_SECTION("Test unquote function")
        EXPECT_EQUAL(argparse::unquote(R"("word")"), "word")
        EXPECT_EQUAL(argparse::unquote("'word'"), "word")
        EXPECT_EQUAL(argparse::unquote("''word''"), "'word'")
        EXPECT_EQUAL(argparse::unquote(R"('word")"), R"('word")")
        EXPECT_EQUAL(argparse::unquote("word"), "word")
    END_SECTION

    START_SECTION("Test boolean options")
        argparse::Option bool_option = argparse::Option().create<bool>('b', "bool", "A boolean");
        EXPECT_EQUAL(bool_option.isValid(), true)
        EXPECT_EQUAL(bool_option.isValid("true"), true)
        EXPECT_EQUAL(bool_option.isValid("TRUE"), true)
        EXPECT_EQUAL(bool_option.isValid("True"), true)
        EXPECT_EQUAL(bool_option.isValid("1"), true)
        EXPECT_EQUAL(bool_option.isValid("0"), true)
        EXPECT_EQUAL(bool_option.isValid("false"), true)
        EXPECT_EQUAL(bool_option.isValid("FALSE"), true)
        EXPECT_EQUAL(bool_option.isValid("False"), true)
        EXPECT_EQUAL(bool_option.isValid("foo"), false)
        EXPECT_EQUAL(bool_option.isValid("2"), false)

        EXPECT_EQUAL(bool_option.setValue("true"), true)
        EXPECT_EQUAL(bool_option.isValid(), true)
        EXPECT_EQUAL(bool_option.setValue("false"), true)
        EXPECT_EQUAL(bool_option.isValid(), true)
        EXPECT_EQUAL(bool_option.setValue("bar"), false)
        EXPECT_EQUAL(bool_option.isValid(), false)

    END_SECTION

    START_SECTION("Test integer options")
        argparse::Option int_option = argparse::Option().create<int>('i', "int", "A integer option");
        EXPECT_EQUAL(int_option.isValid(), true)
        EXPECT_EQUAL(int_option.isValid("0"), true)
        EXPECT_EQUAL(int_option.isValid("+99"), false)
        EXPECT_EQUAL(int_option.isValid("-23"), true)
        EXPECT_EQUAL(int_option.isValid("100"), true)
        EXPECT_EQUAL(int_option.isValid("86.0"), false)
        EXPECT_EQUAL(int_option.isValid("foo"), false)

        EXPECT_EQUAL(int_option.setValue("1"), true)
        EXPECT_EQUAL(int_option.isValid(), true)
        EXPECT_EQUAL(int_option.setValue("42069"), true)
        EXPECT_EQUAL(int_option.isValid(), true)
        EXPECT_EQUAL(int_option.setValue("-8"), true)
        EXPECT_EQUAL(int_option.isValid(), true)
        EXPECT_EQUAL(int_option.setValue("-756"), true)
        EXPECT_EQUAL(int_option.isValid(), true)
        EXPECT_EQUAL(int_option.setValue("bar"), false)
        EXPECT_EQUAL(int_option.isValid(), false)
        EXPECT_EQUAL(int_option.setValue("+3"), false)
        EXPECT_EQUAL(int_option.isValid(), false)

    END_SECTION

    START_SECTION("Test float options")
        argparse::Option float_option = argparse::Option().create<float>('f', "float", "A float option");
        EXPECT_EQUAL(float_option.isValid(), true)
        EXPECT_EQUAL(float_option.isValid("0"), true)
        EXPECT_EQUAL(float_option.isValid("0.19"), true)
        EXPECT_EQUAL(float_option.isValid("+99.2"), false)
        EXPECT_EQUAL(float_option.isValid("-23"), true)
        EXPECT_EQUAL(float_option.isValid("100"), true)
        EXPECT_EQUAL(float_option.isValid("86.0"), true)
        EXPECT_EQUAL(float_option.isValid("420.69"), true)
        EXPECT_EQUAL(float_option.isValid("16."), true)
        EXPECT_EQUAL(float_option.isValid("16."), true)
        EXPECT_EQUAL(float_option.isValid("foo"), false)
        EXPECT_EQUAL(float_option.isValid("25.6.7"), false)

        EXPECT_EQUAL(float_option.setValue("1"), true)
        EXPECT_EQUAL(float_option.isValid(), true)
        EXPECT_EQUAL(float_option.setValue("420.69"), true)
        EXPECT_EQUAL(float_option.isValid(), true)
        EXPECT_EQUAL(float_option.setValue("-8"), true)
        EXPECT_EQUAL(float_option.isValid(), true)
        EXPECT_EQUAL(float_option.setValue("-7.56"), true)
        EXPECT_EQUAL(float_option.isValid(), true)
        EXPECT_EQUAL(float_option.setValue("bar"), false)
        EXPECT_EQUAL(float_option.isValid(), false)
        EXPECT_EQUAL(float_option.setValue("+3"), false)
        EXPECT_EQUAL(float_option.isValid(), false)

    END_SECTION

    START_SECTION("Test string options")
        EXPECT_EXCEPTION(std::invalid_argument, argparse::Option().create<std::string>('-', "", ""))
        EXPECT_EXCEPTION(std::invalid_argument, argparse::Option().create<std::string>('-', "--str", ""))
        EXPECT_EXCEPTION(std::invalid_argument, argparse::Option().create<std::string>('s', "--str", ""))
        EXPECT_EXCEPTION(std::invalid_argument, argparse::Option().create<std::string>('s', "str", "", "true",
                                                                                     argparse::Option::ACTION::STORE_TRUE))
    END_SECTION

    START_SECTION("Test ArgumentParser help")
        argparse::Option::maxLineLen = 80;
        argparse::Option::indendentLen = 22;
        std::string testString = "Print a really interesting and important help message that is more than 1 line long with an indent.";
        std::string expected = "  Print a really interesting and important help message that is more than 1 line\n"
                               "                        long with an indent.";
        // std::string obsd = argparse::Option::multiLineString(testString, 2);
        EXPECT_EQUAL(argparse::Option::multiLineString(testString, 2), expected)
        expected = "Print a really interesting and important help message that is more than 1 line\n"
                   "                      long with an indent.";
        EXPECT_EQUAL(argparse::Option::multiLineString(testString, 0), expected)
        argparse::Option::maxLineLen = 100;
        EXPECT_EQUAL(argparse::Option::multiLineString(testString, 0), testString)
        argparse::Option::maxLineLen = 80;

        // test single line help signature
        argparse::Option helpOption = argparse::Option().create<bool>('h', "help", "Display help message and exit.",
                                                                  "false", argparse::Option::ACTION::HELP);
        expected = "  -h, --help            Display help message and exit.";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // multi line help signature
        helpOption = argparse::Option().create<bool>('h', "help", "Display a really interesting and important help message that has more than 1 line",
                                                   false, argparse::Option::ACTION::HELP);
        expected = "  -h, --help            Display a really interesting and important help message\n"
                   "                        that has more than 1 line";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // 3 line help signature
        std::string longMessage = "Display a really interesting and important help message that has more than 2 "
                                  "lines. It will be the best help message the you will ever see in your life because "
                                  "it is so long and descriptive.";
        helpOption = argparse::Option().create<bool>('h', "help", longMessage, "false", argparse::Option::ACTION::HELP);
        expected = "  -h, --help            Display a really interesting and important help message\n"
                   "                        that has more than 2 lines. It will be the best help\n"
                   "                        message the you will ever see in your life because it is\n"
                   "                        so long and descriptive.";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // 3 line help signature with long flag
        helpOption = argparse::Option().create<int>("thingsToPrint",
                                                  "Display a really interesting and important help message that has more than 1 line.");
        expected = "    --thingsToPrint THINGSTOPRINT\n"
                   "                          Display a really interesting and important help\n"
                   "                          message that has more than 1 line.";
        EXPECT_EQUAL(helpOption.signature(4), expected)

        // edge case
        helpOption = argparse::Option().create<bool>('h', "helpHelpPleasse", "Display help message and exit.",
                                                   "false", argparse::Option::ACTION::HELP);
        expected = "  -h, --helpHelpPleasse Display help message and exit.";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // edge case
        helpOption = argparse::Option().create<bool>('h', "helpHelpPleasse", "Display help message and exit.",
                                                   "false", argparse::Option::ACTION::HELP);
        expected = "-h, --helpHelpPleasse\n"
                   "                    Display help message and exit.";
        argparse::Option::indendentLen = 20;
        EXPECT_EQUAL(helpOption.signature(0), expected)

        // 3 line help signature with long flag and default option
        helpOption = argparse::Option().create<int>("thingsToPrint",
                                                  "Display a really interesting and important help message that has more than 1 line.",
                                                  5);
        expected = "     --thingsToPrint THINGSTOPRINT\n"
                   "                           Display a really interesting and important help\n"
                   "                           message that has more than 1 line. '5' is the\n"
                   "                           default.";
        argparse::Option::indendentLen = 22;
        EXPECT_EQUAL(helpOption.signature(5), expected)

        argparse::PositionalArgument positional = argparse::PositionalArgument().create<std::string>("a_positional_arg", "A positional argument.");
        expected = "  a_positional_arg      A positional argument.";
        std::string obsd = positional.signature(2);
        EXPECT_EQUAL(positional.signature(2), expected)
    END_SECTION

    START_SECTION("Test arg parsing")
        // helper functions
        std::string key, value;
        argparse::ArgumentParser::splitOption("-p", key, value);
        EXPECT_EQUAL(key, "p")
        argparse::ArgumentParser::splitOption("ps", key, value);
        EXPECT_EQUAL(key, "p")
        EXPECT_EQUAL(value, "s")
        argparse::ArgumentParser::splitOption("lth", key, value);
        EXPECT_EQUAL(key, "l")
        EXPECT_EQUAL(value, "th")
        argparse::ArgumentParser::splitOption("--p", key, value);
        EXPECT_EQUAL(key, "")
        argparse::ArgumentParser::splitOption("--help", key, value);
        EXPECT_EQUAL(key, "help")
        argparse::ArgumentParser::splitOption("-help", key, value);
        EXPECT_EQUAL(key, "h")
        EXPECT_EQUAL(value, "elp")
        argparse::ArgumentParser::splitOption("---help", key, value);
        EXPECT_EQUAL(key, "")
        argparse::ArgumentParser::splitOption("-F'\t'", key, value);
        EXPECT_EQUAL(key, "F")
        EXPECT_EQUAL(value, "\t")
        argparse::ArgumentParser::splitOption("-F\t", key, value);
        EXPECT_EQUAL(key, "F")
        EXPECT_EQUAL(value, "\t")
        argparse::ArgumentParser::splitOption("--color=auto", key, value);
        EXPECT_EQUAL(key, "color")
        EXPECT_EQUAL(value, "auto")
        argparse::ArgumentParser::splitOption("--color='auto'", key, value);
        EXPECT_EQUAL(key, "color")
        EXPECT_EQUAL(value, "auto")
        argparse::ArgumentParser::splitOption("--print:spectra", key, value);
        EXPECT_EQUAL(key, "print:spectra")
        EXPECT_EQUAL(value, "")
        argparse::ArgumentParser::splitOption("--out.file='~/code/ionFinder/build/bin/script'", key, value);
        EXPECT_EQUAL(key, "out.file")
        EXPECT_EQUAL(value, "~/code/ionFinder/build/bin/script")

        argparse::ArgumentParser args("A test argument parser.");
        std::vector<std::string> argVector = {std::string(argv[0]), "-h"};
        char** argArray = new char* [TEST_ARGV_SIZE];
        int argCount = populateArgArray(argVector, argArray);
        args.setHelpBehavior(argparse::ArgumentParser::RETURN_FALSE);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)
        args.setHelpBehavior(argparse::ArgumentParser::RETURN_TRUE);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)

        // Test argument order parsing
        args.addArgument<std::string>("source", "Source file(s)", 1, std::string::npos);
        args.addArgument<std::string>("dest", "Destination");
        argVector = {std::string(argv[0]), "file1", "file2", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(args.getArgument("source").getArgCount(), 2)
        EXPECT_EQUAL(args.getArgument("dest").getArgCount(), 1)
        EXPECT_EQUAL(args.getArgumentValue("dest"), "dest")
        EXPECT_EXCEPTION(std::runtime_error, args.getArgumentValue("source"))
        EXPECT_EXCEPTION(std::out_of_range, args.getArgumentValue("foo"))
        argVector = {std::string(argv[0]), "-", "file1", "file2", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)
        args.setSingleDashBehavior(argparse::ArgumentParser::START_POSITIONAL);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)

        argVector = {std::string(argv[0]), "dest"};
        argCount = populateArgArray(argVector, argArray);
        // for(int i = 0; i < argCount; i++) std::cout << argArray[i] << std::endl;
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)

        // test choices
        args.addOption<int>("mode", "The mode.", 1, {1, 2});
        argVector = {std::string(argv[0]), "--mode", "2", "file", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(args.getOptionValue<int>("mode"), 2)
        argVector = {std::string(argv[0]), "--mode", "3", "file", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)
        argparse::Option intChoiceOption = argparse::Option().create("foo", "The mode.", 1, {1, 2});
        EXPECT_EQUAL(intChoiceOption.signature(0), "--foo {1, 2}          The mode. '1' is the default.")
        argparse::Option charChoiceOption = argparse::Option().create("foo", "The mode.", 'a', {'a', 'b', 'c'});
        EXPECT_EQUAL(charChoiceOption.signature(0), "--foo {'a', 'b', 'c'} The mode. 'a' is the default.")

        // Real program test
        argparse::ArgumentParser programArgs("Summarize information in tsv/csv files.");
        programArgs.setHelpBehavior(argparse::ArgumentParser::RETURN_FALSE);
        programArgs.setSingleDashBehavior(argparse::ArgumentParser::START_POSITIONAL);
        programArgs.addOption<int>('n', "", "Number of lines to look for data types.", 5);
        programArgs.addOption<int>('p', "rows", "Number of rows to print.", 1);
        programArgs.addOption<bool>('s', "noHeader", "Don't treat first line as header.",
                                    false, argparse::Option::STORE_TRUE);
        programArgs.addOption<bool>('j', "", "Just another bool option.", false, argparse::Option::ACTION::STORE_TRUE);
        programArgs.addOption<char>('F', "sep", "Field separator.", '\t');
        programArgs.addOption<std::string>('m', "mode", "Program output mode.", "str", {"str", "summary"});
        programArgs.setVersion("v1.0");
        programArgs.addArgument("file", "File to look at. If no file is given, read from stdin.", 0, 1);

        argVector = {std::string(argv[0]), "--help"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), ""};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<int>("rows"), 1)

        argVector = {std::string(argv[0]), "-n", "--rows", "-F" "\t"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "-n", "50", "--rows", "-F", "\t"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "-n", "50", "--rows", "2", "-F", "'\t'"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<int>("n"), 50)
        EXPECT_EQUAL(programArgs.getOptionValue<char>("F"), '\t')
        EXPECT_EQUAL(programArgs.getOptionValue<char>("sep"), '\t')
        EXPECT_EQUAL(programArgs.getOptionValue<int>("rows"), 2)

        argVector = {std::string(argv[0]), "-n", "50", "--rows", "2", "-F", "','", "foo", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("noHeader"), false)
        EXPECT_EQUAL(programArgs.getOptionValue<int>("rows"), 2)
        EXPECT_EQUAL(programArgs.getOptionValue<char>("sep"), ',')

        argVector = {std::string(argv[0]), "-n50", "-F' '", "--noHeader", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("noHeader"), true);
        EXPECT_EQUAL(programArgs.getOptionValue<int>("n"), 50);

        argVector = {std::string(argv[0]), "-n", "50", "-sp", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "-n", "50", "-sj", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("j"), true)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("noHeader"), true)

        argVector = {std::string(argv[0]), "-n", "50", "-sjF", "\t", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("j"), true)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("noHeader"), true)
        EXPECT_EQUAL(programArgs.getOptionValue<char>("sep"), '\t')
        EXPECT_EQUAL(programArgs.getOptionValue<char>("F"), '\t')

        argVector = {std::string(argv[0]), "-n", "50", "-sx", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "-n", "50", "-sjx", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "-n", "50", "--sj", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "-n", "50", "--s", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

        argVector = {std::string(argv[0]), "--rows=50", "-s", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<int>("p"), 50)
        EXPECT_EQUAL(programArgs.getOptionValue<int>("rows"), 50)

        argVector = {std::string(argv[0]), "--version", "-n"};
        argCount = populateArgArray(argVector, argArray);
        programArgs.setVersionBehavior(argparse::ArgumentParser::HELP_VERSION_BEHAVIOR::RETURN_FALSE);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

    delete[] argArray;
    END_SECTION

END_TEST

