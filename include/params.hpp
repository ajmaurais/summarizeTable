
#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <map>
#include <vector>
#include <iostream>
#include <regex>

namespace params {
    class Option {
    public:
        enum VALUE_TYPE {
            STRING, BOOL, INT, FLOAT
        };
        enum ACTION {
            STORE_TRUE, STORE_FALSE, NONE
        };
    private:
        // static const std::regex BOOL_PATTERN;
        // static const std::regex INT_PATTERN;
        // static const std::regex FLOAT_PATTERN;

        std::string _shortOpt;
        std::string _longOpt;
        std::string _name;
        std::string _help;
        std::string _value;

        VALUE_TYPE _valueType;
        ACTION _action;

        //! True if option should be treated as a positional argument
        bool _isPositional;
        //! True if option was set on the command line
        bool _isSet;

    public:
        Option(std::string shortOpt, std::string longOpt, std::string help,
               VALUE_TYPE valueType, std::string defaultVal = "", ACTION action = NONE);

        Option(std::string name, std::string help, VALUE_TYPE valueType);

        bool setValue(std::string = "");

        bool isValid() const;

        bool isValid(std::string) const;

        void printHelp(std::ostream &out = std::cout) {
            out << _help;
        }
    };

    class Params {
    private:
        std::map<std::string, Option> _args;
        std::string _description;

    public:
        explicit Params(std::string description = "") {
            _description = description;
        }
    };
}

#endif
