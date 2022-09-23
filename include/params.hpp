
#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <map>
#include <vector>
#include <iostream>
#include <regex>
#include <filesystem>
#include <algorithm>

namespace params {
    class Option {
    public:
        enum TYPE {
            STRING, BOOL, INT, FLOAT
        };
        enum ACTION {
            // TODO: Add help and version actions.
            STORE_TRUE, STORE_FALSE, HELP, VERSION, NONE
        };

        static size_t maxLineLen;
        static size_t indendentLen;
    private:
        std::string _shortOpt;
        std::string _longOpt;
        std::string _name;
        std::string _help;
        std::string _value;
        std::string _defaultValue;

        TYPE _valueType;
        ACTION _action;

        //! True if option should be treated as a positional argument
        bool _isPositional;
        //! True if option was set on the command line
        bool _isSet;

        void _checkOptFlags() const;
        static bool newWord(char);
    public:
        Option(std::string shortOpt, std::string longOpt, std::string help,
               TYPE valueType, std::string defaultVal = "", ACTION action = NONE);
        Option(std::string name, std::string help, TYPE valueType);
        Option(const Option&);
        Option& operator = (const Option& rhs);
        Option();

        void setName(std::string name) {
            _name = name;
        }
        bool setValue(std::string = "");

        bool isValid() const;
        bool isValid(std::string) const;
        std::string getName() const {
            return _name;
        }
        ACTION getAction() const {
            return _action;
        }
        TYPE getType() const {
            return _valueType;
        }

        std::string signature(int indent = 0) const;
        std::string help() const;
        static std::string parseOption(std::string arg);
        static std::string multiLineString(std::string addStr, size_t indent, bool indentFirstLine = false);
    };

    class Params {
    private:
        std::map<std::string, Option> _options;
        std::vector<Option> _args;
        std::string _description;
        std::string _programName;
        bool _help;
        std::string _version;

        static bool isFlag(std::string arg) {
            return !arg.empty() && arg[0] == '-';
        }
        bool isOption(std::string arg) const;
    public:
        explicit Params(std::string description = "", std::string programName = "", bool help = true) {
            _description = description;
            _programName = programName;
            _help = help;
            if(_help) addOption("h", "help", "Show help and exit.",
                                Option::TYPE::BOOL, "false", Option::ACTION::HELP);
        }

        void addOption(std::string shortOpt, std::string longOpt, std::string help,
                       Option::TYPE valueType, std::string defaultVal = "",
                       Option::ACTION action = Option::ACTION::NONE);
        void setVersion(std::string version, std::string shortOpt = "v", std::string longOpt = "version");
        void addArgument(std::string name, std::string help,
                         Option::TYPE valueType = Option::TYPE::STRING);
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
