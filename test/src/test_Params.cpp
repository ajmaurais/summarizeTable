//
// Created by Aaron Maurais on 9/21/22.
//

#include <iostream>

#include <testing.hpp>
#include <params.hpp>

int const TEST_ARGV_SIZE = 256;

int populateArgArray(const std::vector<std::string>& argVector, char* argArray[TEST_ARGV_SIZE]) {
    for(size_t i = 0; i < argVector.size(); i++) {
        argArray[i] = new char[argVector[i].size()];
        strcpy(argArray[i], argVector[i].c_str());
    }
    return argVector.size();
}

START_TEST("Params")
    START_SECTION("Test unquote function")
        EXPECT_EQUAL(params::unquote(R"("word")"), "word")
        EXPECT_EQUAL(params::unquote("'word'"), "word")
        EXPECT_EQUAL(params::unquote("''word''"), "'word'")
        EXPECT_EQUAL(params::unquote(R"('word")"), R"('word")")
        EXPECT_EQUAL(params::unquote("word"), "word")
    END_SECTION

    START_SECTION("Test boolean options")
        params::Option bool_option('b', "bool", "A boolean ", params::Option::TYPE::BOOL);
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

        EXPECT_EXCEPTION(std::invalid_argument, params::Option('b', "", "", params::Option::TYPE::INT, "", params::Option::ACTION::STORE_FALSE))
        EXPECT_EXCEPTION(std::invalid_argument, params::Option('b', "", "", params::Option::TYPE::BOOL, "foo"))
    END_SECTION

    START_SECTION("Test integer options")
        params::Option int_option('i', "int", "A integer option", params::Option::TYPE::INT);
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

        EXPECT_EXCEPTION(std::invalid_argument, params::Option('i', "", "", params::Option::TYPE::INT, "fart"))
        EXPECT_EXCEPTION(std::invalid_argument, params::Option('i', "", "", params::Option::TYPE::INT, "0.16"))
    END_SECTION

    START_SECTION("Test float options")
        params::Option float_option('f', "float", "A float option", params::Option::TYPE::FLOAT);
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

        EXPECT_EXCEPTION(std::invalid_argument, params::Option('i', "", "", params::Option::TYPE::FLOAT, "fart"))
    END_SECTION

    START_SECTION("Test string options")
        EXPECT_EXCEPTION(std::invalid_argument, params::Option('-', "", "", params::Option::TYPE::STRING))
        EXPECT_EXCEPTION(std::invalid_argument, params::Option('-', "--str", "", params::Option::TYPE::STRING))
        EXPECT_EXCEPTION(std::invalid_argument, params::Option('s', "--str", "", params::Option::TYPE::STRING))
        EXPECT_EXCEPTION(std::invalid_argument, params::Option('s', "str", "", params::Option::TYPE::STRING,
                                                               "true", params::Option::ACTION::STORE_TRUE))
    END_SECTION

    START_SECTION("Test Params help")
        params::Option::maxLineLen = 80;
        params::Option::indendentLen = 22;
        std::string testString = "Print a really interesting and important help message that is more than 1 line long with an indent.";
        std::string expected = "  Print a really interesting and important help message that is more than 1 line\n"
                               "                        long with an indent.";
        // std::string obsd = params::Option::multiLineString(testString, 2);
        EXPECT_EQUAL(params::Option::multiLineString(testString, 2), expected)
        expected = "Print a really interesting and important help message that is more than 1 line\n"
                   "                      long with an indent.";
        EXPECT_EQUAL(params::Option::multiLineString(testString, 0), expected)
        params::Option::maxLineLen = 100;
        EXPECT_EQUAL(params::Option::multiLineString(testString, 0), testString)
        params::Option::maxLineLen = 80;

        // test single line help signature
        params::Option helpOption('h', "help", "Display help message and exit.",
                                  params::Option::TYPE::BOOL, "false", params::Option::ACTION::HELP);
        expected = "  -h, --help            Display help message and exit.";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // multi line help signature
        helpOption = params::Option('h', "help", "Display a really interesting and important help message that has more than 1 line",
                                    params::Option::TYPE::BOOL, "false", params::Option::ACTION::HELP);
        expected = "  -h, --help            Display a really interesting and important help message\n"
                   "                        that has more than 1 line";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // 3 line help signature
        std::string longMessage = "Display a really interesting and important help message that has more than 2 "
                                  "lines. It will be the best help message the you will ever see in your life because "
                                  "it is so long and descriptive.";
        helpOption = params::Option('h', "help", longMessage,
                                    params::Option::TYPE::BOOL, "false", params::Option::ACTION::HELP);
        expected = "  -h, --help            Display a really interesting and important help message\n"
                   "                        that has more than 2 lines. It will be the best help\n"
                   "                        message the you will ever see in your life because it is\n"
                   "                        so long and descriptive.";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // 3 line help signature with long flag
        helpOption = params::Option("thingsToPrint",
                                    "Display a really interesting and important help message that has more than 1 line.",
                                    params::Option::TYPE::INT, "");
        expected = "    --thingsToPrint THINGSTOPRINT\n"
                   "                          Display a really interesting and important help\n"
                   "                          message that has more than 1 line.";
        EXPECT_EQUAL(helpOption.signature(4), expected)

        // edge case
        helpOption = params::Option('h', "helpHelpPleasse", "Display help message and exit.",
                                   params::Option::TYPE::BOOL, "false", params::Option::ACTION::HELP);
        expected = "  -h, --helpHelpPleasse Display help message and exit.";
        EXPECT_EQUAL(helpOption.signature(2), expected)

        // edge case
        helpOption = params::Option('h', "helpHelpPleasse", "Display help message and exit.",
                                    params::Option::TYPE::BOOL, "false", params::Option::ACTION::HELP);
        expected = "-h, --helpHelpPleasse\n"
                   "                    Display help message and exit.";
        params::Option::indendentLen = 20;
        EXPECT_EQUAL(helpOption.signature(0), expected)

        // 3 line help signature with long flag and default option
        helpOption = params::Option("thingsToPrint",
                                    "Display a really interesting and important help message that has more than 1 line.",
                                    params::Option::TYPE::INT, "5");
        expected = "     --thingsToPrint THINGSTOPRINT\n"
                   "                           Display a really interesting and important help\n"
                   "                           message that has more than 1 line. '5' is the\n"
                   "                           default.";
        params::Option::indendentLen = 22;
        EXPECT_EQUAL(helpOption.signature(5), expected)

        params::PositionalArgument positional("a_positional_arg", "A positional argument.", params::Option::TYPE::STRING);
        expected = "  a_positional_arg      A positional argument.";
        std::string obsd = positional.signature(2);
        EXPECT_EQUAL(positional.signature(2), expected)
    END_SECTION

    START_SECTION("Test arg parsing")
        // helper functions
        std::string key, value;
        params::Option::parseOption("-p", key, value);
        EXPECT_EQUAL(key, "p")
        params::Option::parseOption("--p", key, value);
        EXPECT_EQUAL(key, "p")
        params::Option::parseOption("--help", key, value);
        EXPECT_EQUAL(key, "help")
        params::Option::parseOption("-help", key, value);
        EXPECT_EQUAL(key, "h")
        EXPECT_EQUAL(value, "elp")
        params::Option::parseOption("---help", key, value);
        EXPECT_EQUAL(key, "-help")
        params::Option::parseOption("-F'\t'", key, value);
        EXPECT_EQUAL(key, "F")
        EXPECT_EQUAL(value, "\t")
        params::Option::parseOption("-F\t", key, value);
        EXPECT_EQUAL(key, "F")
        EXPECT_EQUAL(value, "\t")

        params::Params args("A test argument parser.");
        EXPECT_EXCEPTION(std::invalid_argument, args.addArgument("a positional argument", "A positional argument."))
        std::vector<std::string> argVector = {std::string(argv[0]), "-h"};
        char** argArray = new char* [TEST_ARGV_SIZE];
        int argCount = populateArgArray(argVector, argArray);
        args.setHelpBehavior(params::Params::RETURN_FALSE);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)
        args.setHelpBehavior(params::Params::RETURN_TRUE);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)

        // Test argument order parsing
        args.addArgument("source", "Source file(s)", 1, std::string::npos);
        args.addArgument("dest", "Destination");
        argVector = {std::string(argv[0]), "file1", "file2", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(args.getArgumentValues("source").size(), 2)
        EXPECT_EQUAL(args.getArgumentValues("dest").size(), 1)
        EXPECT_EQUAL(args.getArgumentValue("dest"), "dest")
        EXPECT_EXCEPTION(std::runtime_error, args.getArgumentValues<int>("source"))
        EXPECT_EXCEPTION(std::runtime_error, args.getArgumentValue("source"))
        EXPECT_EXCEPTION(std::out_of_range, args.getArgumentValues("foo"))
        EXPECT_EXCEPTION(std::out_of_range, args.getArgumentValue("foo"))
        argVector = {std::string(argv[0]), "-", "file1", "file2", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)
        args.setSingleDashBehavior(params::Params::START_POSITIONAL);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)

        argVector = {std::string(argv[0]), "dest"};
        argCount = populateArgArray(argVector, argArray);
        // for(int i = 0; i < argCount; i++) std::cout << argArray[i] << std::endl;
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)

        // test choices
        EXPECT_EXCEPTION(std::invalid_argument, args.addOption("mode1", "The mode.", params::Option::INT, "1", {"1", "foo"}))
        EXPECT_EXCEPTION(std::invalid_argument, args.addOption("mode2", "The mode.", params::Option::INT, "1", {"1", "2", "2"}))
        EXPECT_EXCEPTION(std::invalid_argument, args.addOption("mode3", "The mode.", params::Option::INT, "3", {"1", "2"}))
        EXPECT_EXCEPTION(std::invalid_argument, args.addOption("mode4", "The mode.", params::Option::BOOL, "1", {"0", "1"}))
        args.addOption("mode", "The mode.", params::Option::INT, "1", {"1", "2"});
        argVector = {std::string(argv[0]), "--mode", "2", "file", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(args.getOptionValue<int>("mode"), 2)
        argVector = {std::string(argv[0]), "--mode", "3", "file", "dest"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(args.parseArgs(argCount, argArray), false)
        params::Option intChoiceOption("foo", "The mode.", params::Option::INT, "1", {"1", "2"});
        EXPECT_EQUAL(intChoiceOption.signature(0), "--foo {1, 2}          The mode. '1' is the default.")
        params::Option charChoiceOption("foo", "The mode.", params::Option::CHAR, "a", {"a", "b", "c"});
        EXPECT_EQUAL(charChoiceOption.signature(0), "--foo {'a', 'b', 'c'} The mode. 'a' is the default.")

        // Real program test
        params::Params programArgs("Summarize information in tsv/csv files.");
        programArgs.setHelpBehavior(params::Params::RETURN_FALSE);
        programArgs.setSingleDashBehavior(params::Params::START_POSITIONAL);
        programArgs.addOption('n', "", "Number of lines to look for data types.", params::Option::TYPE::INT, "5");
        programArgs.addOption('p', "rows", "Number of rows to print.", params::Option::TYPE::INT, "1");
        programArgs.addOption('s', "noHeader", "Don't treat first line as header.",
                       params::Option::BOOL, "false", params::Option::STORE_TRUE);
        programArgs.addOption('F', "sep", "Field separator.", params::Option::CHAR, "\t");
        programArgs.addOption('m', "mode", "Program output mode.", params::Option::STRING, "str", {"str", "summary"});
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

        argVector = {std::string(argv[0]), "-n50", "-F' '", "--noHeader", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), true)
        EXPECT_EQUAL(programArgs.getOptionValue<bool>("noHeader"), true);
        EXPECT_EQUAL(programArgs.getOptionValue<int>("n"), 50);

        argVector = {std::string(argv[0]), "-n", "50", "-sp", "bar"};
        argCount = populateArgArray(argVector, argArray);
        EXPECT_EQUAL(programArgs.parseArgs(argCount, argArray), false)

    delete[] argArray;
    END_SECTION

END_TEST
