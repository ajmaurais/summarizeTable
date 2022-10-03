
#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <type_traits>
#include <variant>
// #include <function>

namespace params {
    const size_t MAX_LINE_LEN = 100;
    const size_t INDENT_LEN = 22;

    std::string multiLineString(std::string addStr, size_t margin,
                                size_t maxLineLen, size_t indentLen,
                                bool indentFirstLine);
    bool newWord(char c);
    std::string escapeEscapeCharacters(const std::string&);
    std::string unquote(std::string);

    class ArgumentValue {
    public:
        enum VALUE_TYPE {BOOL = 0, INT = 1, LONG = 2, SIZE_T = 3,
            FLOAT = 4, DOUBLE = 5, CHAR = 6, STRING = 7};
        static std::string valueTypeToStr(VALUE_TYPE type);
        typedef std::variant<bool,          // 0
                             int,           // 1
                             long,          // 2
                             size_t,        // 3
                             float,         // 4
                             double,        // 5
                             char,          // 6
                             std::string    // 7
                             > ValueType;
    private:
        ValueType _value;
        bool _isSet;

        static void toType(bool&, const std::string&);
        static void toType(int&, const std::string&);
        static void toType(long&, const std::string&);
        static void toType(size_t&, const std::string&);
        static void toType(float&, const std::string&);
        static void toType(double&, const std::string&);
        static void toType(char&, const std::string&);

        template <typename T> void _setValue(std::string value) {
            T dest;
            toType(dest, value);
            setValue<T>(dest);
        }
    public:
        ArgumentValue() {}
        template <typename T> ArgumentValue(T value) {
            setValue<T>(value);
        }

        template <typename T> void setValue(){
            _value = T();
        }
        template <typename T> void setValue(T value){
            _value = value;
            _isSet = true;
        }
        void setValue(std::string value);

        template <typename T> bool isValid() const {
            return std::holds_alternative<T>(_value);
        }
        bool isValid(const std::string&) const;
        template <typename T> T getValue() const {
            return std::get<T>(_value);
        }
        std::string getValue() const {
            return getValue<std::string>();
        }
        bool isSet() const {
            return _isSet;
        }
        std::string str() const;
        std::string valueTypeStr() const;

        struct Compare {
            bool operator () (const ArgumentValue& lhs, const ArgumentValue& rhs) const {
                return lhs.str() < rhs.str();
            }
        };
    };

    class Argument {
    public:
        enum TYPE {
            STRING, CHAR, BOOL, INT, FLOAT
        };
        static std::string typeToStr(TYPE);
        static bool isNumeric(TYPE);
        template <typename T> static TYPE getValueType() {
            if(std::is_same<bool, T>()) return TYPE::BOOL;
            if(std::is_same<char, T>()) return TYPE::CHAR;
            if(std::is_integral<T>()) return TYPE::INT;
            if(std::is_floating_point<T>()) return TYPE::FLOAT;
            if(std::is_same<std::string, T>()) return TYPE::STRING;
            throw std::runtime_error("Unsupported argument type!");
        }

        //! Argument names must begin with a letter and have only alphanumeric characters, dash or underscore after.
        static std::string validArgNamePattern;
        static size_t maxLineLen;
        static size_t indendentLen;
        // void toType(const std::string&, char&) const;
        // void toType(const std::string&, bool&) const;
        // void toType(const std::string&, int&) const;
        // void toType(const std::string&, float&) const;
    protected:
        std::string _name;
        std::string _help;

        TYPE _valueType;

        //! True if option was set on the command line
        bool _isSet;

        std::string signature(std::string ret, int indent) const;
        static bool isValidName(std::string);
        void _checkValidName() const;
        // template <typename T> static constexpr
    public:
        Argument(const Argument&);
        Argument& operator = (const Argument& rhs);
        Argument();

        void setName(std::string name) {
            _name = name;
            _checkValidName();
        }

        virtual bool isValid() const = 0;
        std::string getName() const {
            return _name;
        }
        TYPE getType() const {
            return _valueType;
        }
        bool isSet() const {
            return _isSet;
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
        // bool isValid(std::string) const override;
    private:
        char _shortOpt;
        std::string _longOpt;
        ArgumentValue _defaultValue;
        ACTION _action;
        ArgumentValue _value;
        std::set<ArgumentValue, ArgumentValue::Compare> _choices;

        template<typename T>
        void _initialize(char shortOpt, std::string longOpt, std::string help,
                         T defaultVal, const std::vector<T>& choices, Option::ACTION action) {
            _shortOpt = shortOpt;
            _longOpt = longOpt;
            _checkOptFlags();
            _name = _longOpt.empty() ? std::string(1, _shortOpt) : _longOpt;
            _help = help;
            _isSet = false;
            _valueType = getValueType<T>();

            _defaultValue.setValue<T>(defaultVal);
            _value.setValue<T>(defaultVal);

            _action = action;
            if(_action != ACTION::NONE && _defaultValue.isSet()) {
                if(_action == ACTION::STORE_TRUE) _value.setValue<bool>(false);
                if(_action == ACTION::STORE_FALSE) _value.setValue<bool>(true);
            }
            if(!isValid()) throw std::invalid_argument("Invalid option!");

            // validate default arg if necessary
            // if (defaultVal.isSet() && !isValid(defaultVal))
            //     throw std::invalid_argument(defaultVal + " is an invalid value for TYPE!");
            if(!choices.empty()) {
                for(const auto& choice: choices) {
                    // if(!_value.isValid(choice))
                    //     throw std::invalid_argument(choice + " is an invalid value for TYPE!");
                    _choices.emplace(choice);
                }
                if (_valueType == BOOL)
                    throw std::invalid_argument("Not able to have choices for TYPE::BOOL");
                // _choices = std::set<std::string>(choices.begin(), choices.end());
                if (_choices.size() != choices.size())
                    throw std::invalid_argument("Options for option choices must be unique!");
                if (_choices.find(_defaultValue) == _choices.end())
                    throw std::invalid_argument("defaultValue of: '" + _defaultValue.str() + "' is not a valid choice!");
            }
            // _defaultValue = defaultVal;
            // _value = defaultVal;
        }
        void _checkOptFlags() const;
    public:
        Option() : Argument() {
            _action = ACTION::NONE;
            _shortOpt = '\0';
        }
        template<typename T>
        Option& create(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal, const std::vector<T>& choices) {
            _initialize<T>(shortOpt, longOpt, help, defaultVal, choices, NONE);
            return *this;
        }
        template<typename T>
        Option& create(std::string longOpt, std::string help,
                       T defaultVal, const std::vector<std::string>& choices) {
            _initialize<T>('\0', longOpt, help, defaultVal, choices, NONE);
            return *this;
        }
        template <typename T>
        Option& create(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal = T(), ACTION action = NONE) {
            _initialize<T>(shortOpt, longOpt, help, defaultVal, std::vector<T>(), action);
            return *this;
        }
        template <typename T>
        Option& create(std::string longOpt, std::string help,
                       T defaultVal, std::vector<T>(), ACTION action = NONE) {
            _initialize<T>('\0', longOpt, help, defaultVal, action);
            return *this;
        }
        Option(const Option&);
        Option& operator = (const Option& rhs);

        bool setValue(std::string value);
        void unsetValue();

        ACTION getAction() const { return _action; }
        // bool isValid(std::string) const override;
        bool isValid() const override;
        std::string help() const override;
        std::string signature(int margin) const override;
        std::string getValue() const { return _value.getValue(); };
        template <typename T> T getValue() const {
            // T temp;
            // toType(_value, temp);
            // return temp;
            return _value.getValue<T>();
        }

        static void parseOption(std::string arg, std::string& option, std::string& value);
    };

    class PositionalArgument : public Argument {
    private:
        std::vector<ArgumentValue> _values;
        size_t _minValues, _maxValues;

    public:
        PositionalArgument() : Argument() {
            _minValues = 1;
            _maxValues = 1;
        }
        template <typename T>
        PositionalArgument& create(std::string name, std::string help,
                                  size_t minValues = 1, size_t maxValues = 1) {
            _name = name;
            _checkValidName();
            _help = help;
            _valueType = getValueType<T>();
            _minValues = minValues;
            if(maxValues < 1) throw std::invalid_argument("Can not have less than 1 maxValue for PositionalArgument");
            _maxValues = maxValues;
            return *this;
        }

        void addValue(std::string value) {
            if(_values.back().isValid(value)) {
                _isSet = true;
            }
            _values.emplace_back();
            _values.back().setValue(value);
        }
        void unsetValues();

        size_t getMinArgs() const { return _minValues; }
        size_t getMaxArgs() const { return _maxValues; }
        bool isValid() const override;
        std::string invalidReason() const;
        std::string signature(int margin) const override;

        // access to argument values
        // std::vector<std::string> getValues() const;

        //! Number of arguments that were set on the command line.
        size_t getArgCount() const {
            return _values.size();
        }
        std::vector<ArgumentValue>::const_iterator begin() const {
            return _values.begin();
        }
        std::vector<ArgumentValue>::const_iterator end() const {
            return _values.end();
        }
        template <typename T> T at(size_t i) const {
            return _values.at(i).getValue<T>();
        }
        std::string getValue() const;
        template <typename T> T getValue() const {
            T ret;
            toType(getValue(), ret);
            return ret;
        }
        // template <typename T> std::vector<T> getValues() const {
        //     std::vector<T> ret;
        //     T temp;
        //     for(const auto& value: _values) {
        //         toType(value, temp);
        //         ret.push_back(temp);
        //     }
        //     return ret;
        // }
    };

    class Params {
    public:
        //! Behavior for when a single dash ('-') is given as an argument.
        enum SINGLE_DASH {
            START_POSITIONAL, ERROR
        };

        //! Behavior for Option::ACTION::HELP or Option::ACTION::VERSION
        enum HELP_VERSION_BEHAVIOR {
            CONTINUE, //! Continue parsing arguments as normal
            RETURN_TRUE, //! return true from parseArgs function.
            RETURN_FALSE, //! return false from parseArgs function.
            EXIT_1, //! Exit with return code of 1.
            EXIT_0 //! Exit with return code of 0.
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
        HELP_VERSION_BEHAVIOR _helpBehavior;
        HELP_VERSION_BEHAVIOR _versionBehavior;

        static bool isFlag(std::string arg) {
            return !arg.empty() && arg[0] == '-';
        }
        bool _isOption(std::string arg) const;
        bool _parsePositionalArgs(int, int, char**);
        void _validatePositionalArgs() const;
        params::PositionalArgument* _nextArg(size_t&, bool);
        bool _doOptionAction(Option::ACTION, bool&) const;
        template <typename T>
        void _addOption(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal, const std::vector<T>& choices, Option::ACTION action) {
            params::Option option = (choices.empty() ? params::Option().create<T>(shortOpt, longOpt, help, defaultVal, action):
                                                       params::Option().create<T>(shortOpt, longOpt, help, defaultVal, choices));
            std::string name = option.getName();
            _options[name] = option;
            _optionOrder.push_back(name);

            // add option to _optionKeys map
            std::string flags[2] = {(shortOpt == '\0' ? "" : std::string(1, shortOpt)), longOpt};
            for(auto & flag : flags) {
                if (!flag.empty()) {
                    if(_optionKeys.find(flag) != _optionKeys.end())
                        throw std::runtime_error("'" + flag + "' already exists as an option!");
                    _optionKeys[flag] = name;
                }
            }
        }
    public:
        explicit Params(std::string description = "", std::string programName = "", bool help = true) {
            _description = description;
            _programName = programName;
            _help = help;
            _helpMargin = 2;
            _singleDashBehavior = ERROR;
            _helpBehavior = EXIT_0;
            _versionBehavior = EXIT_0;
            if(_help) addOption('h', "help", "Show help and exit.", false, Option::ACTION::HELP);
        }

        template <typename T>
        void addOption(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal = T(), Option::ACTION action = Option::ACTION::NONE) {
            _addOption<T>(shortOpt, longOpt, help, defaultVal, std::vector<T>(), action);
        }
        template <typename T>
        void addOption(std::string longOpt, std::string help,
                       T defaultVal = T(), Option::ACTION action = Option::ACTION::NONE) {
            _addOption<T>('\0', longOpt, help, defaultVal, std::vector<bool>(), action);
        }
        template <typename T> void addOption(char shortOpt, std::string longOpt, std::string help,
                                             T defaultVal, const std::vector<T>& choices) {
            _addOption<T>(shortOpt, longOpt, help, defaultVal, choices, Option::NONE);
        }
        template <typename T> void addOption(std::string longOpt, std::string help,
                                             T defaultVal, const std::vector<T>& choices) {
            _addOption<T>('\0', longOpt, help, defaultVal, choices, Option::NONE);
        }
        void setVersion(std::string version, char shortOpt = 'v', std::string longOpt = "version");
        template <typename T>
        void addArgument(std::string name, std::string help,
                         size_t minArgs = 1, size_t maxArgs = 1) {
            if(_args.find(name) != _args.end())
                throw std::runtime_error("'" + name + "' is already a positional argument");
            _args[name] = params::PositionalArgument().create<T>(name, help, minArgs, maxArgs);
            _argOrder.push_back(name);
        }
        void setHelpMargin(int margin) {
            _helpMargin = margin;
        }
        void setSingleDashBehavior(SINGLE_DASH behavior) {
            _singleDashBehavior = behavior;
        }
        void setHelpBehavior(HELP_VERSION_BEHAVIOR behavior) {
            _helpBehavior = behavior;
        }
        void setVersionBehavior(HELP_VERSION_BEHAVIOR behavior) {
            _versionBehavior = behavior;
        }
        bool parseArgs(int, char**);
        void clearArgs();

        std::string signature() const;
        void printHelp() const;
        void usage(std::ostream& = std::cerr) const;
        std::string getVersion() const {
            return _version;
        }
        void printVersion() const;

        // get option/arg values
        bool checkPositionalArgs() const;
        std::string getOptionValue(std::string option) const;
        bool optionIsSet(const std::string& option) const {
            return _options.at(_optionKeys.at(option)).isSet();
        }
        template <typename T> T getOptionValue(const std::string& option) const {
            return _options.at(_optionKeys.at(option)).getValue<T>();
        }

        std::string getArgumentValue(const std::string& argName) const {
            return _args.at(argName).getValue();
        }
        template <typename T> T getArgumentValue(const std::string& argName) const {
            return _args.at(argName).getValue<T>();
        }
        // std::vector<std::string> getArgumentValues(std::string argName) const;
        size_t nArgs(std::string argName) const;
        size_t nPositionalArgs() const { return _args.size(); }
        const PositionalArgument& getArgument(std::string name) {
            return _args.at(name);
        }
        // template <typename T> std::vector<T> getArgumentValues(const std::string& argName) const {
        //     // return _args.at(argName).getValues<T>();
        // }
        //! Get the number of arguments that were given for \p argName on the command line.
        // size_t getArgumentCount(const std::string& argName) const {
        //     return _args.at(argName).getArgCount();
        // }
    };
}

#endif
