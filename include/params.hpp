
#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <map>
#include <vector>
#include <iostream>
#include <regex>
#include <filesystem>
#include <algorithm>

namespace params {
    const size_t MAX_LINE_LEN = 80;
    const size_t INDENT_LEN = 22;

    std::string multiLineString(std::string addStr, size_t margin,
                                size_t maxLineLen, size_t indentLen,
                                bool indentFirstLine);
    bool newWord(char c);
    std::string escapeEscapeCharacters(const std::string&);

    class Argument {
    public:
        enum TYPE {
            STRING, BOOL, INT, FLOAT
        };

        //! Argument names must begin with a letter and have only alphanumeric characters, dash or underscore after.
        static std::string validArgNamePattern;
        static size_t maxLineLen;
        static size_t indendentLen;
    protected:
        std::string _name;
        std::string _help;
        std::string _value;

        TYPE _valueType;

        //! True if option was set on the command line
        bool _isSet;

        std::string signature(std::string ret, int indent) const;
        static bool isValidName(std::string);
        void _checkValidName() const;
    public:
        Argument(const Argument&);
        Argument& operator = (const Argument& rhs);
        Argument();

        void setName(std::string name) {
            _name = name;
            _checkValidName();
        }
        virtual bool setValue(std::string = "") = 0;

        virtual bool isValid() const = 0;
        virtual bool isValid(std::string) const;
        std::string getName() const {
            return _name;
        }
        TYPE getType() const {
            return _valueType;
        }

        virtual std::string help() const;
        virtual std::string signature(int) const = 0;
        static std::string multiLineString(std::string addStr, size_t margin, bool indentFirstLine = false);
    };

    class Option : public Argument {
    public:
        enum ACTION {
            STORE_TRUE, STORE_FALSE, HELP, VERSION, NONE
        };
    private:
        std::string _shortOpt;
        std::string _longOpt;
        std::string _defaultValue;
        ACTION _action;

        void _checkOptFlags() const;
    public:
        Option() : Argument() {
            _action = ACTION::NONE;
        }
        Option(std::string shortOpt, std::string longOpt, std::string help,
               TYPE valueType, std::string defaultVal = "", ACTION action = NONE);
        Option(const Option&);
        Option& operator = (const Option& rhs);

        bool setValue(std::string value) override;

        ACTION getAction() const {
            return _action;
        }
        bool isValid(std::string) const override;
        bool isValid() const override;
        std::string help() const override;
        std::string signature(int margin = 0) const override;

        static std::string parseOption(std::string arg);
    };

    class PositionalArgument : public Argument {
    public:
        PositionalArgument() : Argument() {}
        PositionalArgument(std::string name, std::string help, TYPE valueType);

        bool setValue(std::string) override;

        bool isValid() const override;
        std::string signature(int margin = 0) const override;
    };

    class Params {
    public:
        //! behavior for when a single dash ('-') is given as an argument.
        enum SINGLE_DASH {
            START_POSITIONAL, ERROR
        };
    private:
        // arguments and options
        std::map<std::string, Option> _options;
        std::vector<std::string> _optionOrder;
        std::map<std::string, std::string> _optionKeys;
        std::map<std::string, PositionalArgument> _args;
        std::vector<std::string> _argOrder;

        std::string _description;
        std::string _programName;
        bool _help;
        std::string _version;
        int _helpMargin;
        SINGLE_DASH _singleDashBehavior;

        static bool isFlag(std::string arg) {
            return !arg.empty() && arg[0] == '-';
        }
        bool isOption(std::string arg) const;
        bool parsePositionalArgs(int, int, char**);
    public:
        explicit Params(std::string description = "", std::string programName = "", bool help = true) {
            _description = description;
            _programName = programName;
            _help = help;
            _helpMargin = 2;
            _singleDashBehavior = ERROR;
            if(_help) addOption("h", "help", "Show help and exit.", Option::TYPE::BOOL, "false", Option::ACTION::HELP);
        }

        void addOption(std::string shortOpt, std::string longOpt, std::string help,
                       Option::TYPE valueType, std::string defaultVal = "",
                       Option::ACTION action = Option::ACTION::NONE);
        void setVersion(std::string version, std::string shortOpt = "v", std::string longOpt = "version");
        void addArgument(std::string name, std::string help,
                         Option::TYPE valueType = Option::TYPE::STRING);
        void setHelpMargin(int margin) {
            _helpMargin = margin;
        }
        void setSingleDashBehavior(SINGLE_DASH behavior) {
            _singleDashBehavior = behavior;
        }
        bool parseArgs(int, char**);

        std::string signature() const;
        void printHelp() const;
        void usage(std::ostream& = std::cerr) const;
        std::string getVersion() const {
            return _version;
        }
        void printVersion() const;
    };
}

#endif
