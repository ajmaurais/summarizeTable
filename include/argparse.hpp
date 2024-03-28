
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

namespace argparse {
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
        static bool isValid(VALUE_TYPE, const std::string&);
        template <typename T> static VALUE_TYPE templateToType() {
            if(std::is_same<T, bool>()) return VALUE_TYPE::BOOL;
            if(std::is_same<T, int>()) return VALUE_TYPE::INT;
            if(std::is_same<T, long>()) return VALUE_TYPE::LONG;
            if(std::is_same<T, size_t>()) return VALUE_TYPE::SIZE_T;
            if(std::is_same<T, float>()) return VALUE_TYPE::FLOAT;
            if(std::is_same<T, double>()) return VALUE_TYPE::DOUBLE;
            if(std::is_same<T, char>()) return VALUE_TYPE::CHAR;
            if(std::is_same<T, std::string>()) return VALUE_TYPE::STRING;
            throw std::invalid_argument("Unknown template!");
        }
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
        ArgumentValue() {
            _isSet = false;
        }
        template <typename T> ArgumentValue(T value) {
            setValue<T>(value);
        }
        explicit ArgumentValue(VALUE_TYPE type);

        template <typename T> void setValue(){
            _value = T();
            _isSet = false;
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
        static size_t maxLineLen;
        static size_t indendentLen;
    protected:
        std::string _name;
        std::string _help;

        TYPE _valueType;
        ArgumentValue::VALUE_TYPE _templateType;

        //! True if option was set on the command line
        bool _isSet;

        std::string signature(std::string ret, int indent) const;
    public:
        Argument(const Argument&);
        Argument& operator = (const Argument& rhs);
        Argument();

        void setName(std::string name) {
            _name = name;
        }

        // virtual bool isValid() const = 0;
        std::string getName() const {
            return _name;
        }
        TYPE getType() const {
            return _valueType;
        }
        bool isValid(std::string s) const;
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

        static std::string validLongOptionPattern;
        static std::string validShortOptionPattern;
    private:
        char _shortOpt;
        std::string _longOpt;
        ArgumentValue _defaultValue;
        ACTION _action;
        ArgumentValue _value;
        std::set<ArgumentValue, ArgumentValue::Compare> _choices;

        template<typename T>
        void _initialize(char shortOpt, std::string longOpt, std::string help, bool hasDefault,
                         T defaultVal, const std::vector<T>& choices, Option::ACTION action) {
            _shortOpt = shortOpt;
            _longOpt = longOpt;
            _checkOptFlags();
            _name = _longOpt.empty() ? std::string(1, _shortOpt) : _longOpt;
            _help = help;
            _isSet = false;
            _valueType = getValueType<T>();
            _templateType = ArgumentValue::templateToType<T>();

            if(hasDefault) _defaultValue.setValue<T>(defaultVal);
            else _defaultValue.setValue<T>();
            _value.setValue<T>(defaultVal);

            _action = action;
            if(_action != ACTION::NONE && _defaultValue.isSet()) {
                if(_action == ACTION::STORE_TRUE) _value.setValue<bool>(false);
                if(_action == ACTION::STORE_FALSE) _value.setValue<bool>(true);
            }
            if(!isValid()) throw std::invalid_argument("Invalid option!");

            if(!choices.empty()) {
                for(const auto& choice: choices) {
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
        }
        void _unset();
        void _checkOptFlags() const;
    public:
        Option() : Argument() {
            _action = ACTION::NONE;
            _shortOpt = '\0';
        }
        template<typename T>
        Option& create(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal, const std::vector<T>& choices) {
            _initialize<T>(shortOpt, longOpt, help, true, defaultVal, choices, NONE);
            return *this;
        }
        template<typename T>
        Option& create(std::string longOpt, std::string help,
                       T defaultVal, const std::vector<T>& choices) {
            _initialize<T>('\0', longOpt, help, true, defaultVal, choices, NONE);
            return *this;
        }
        template <typename T>
        Option& create(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal, ACTION action = NONE) {
            _initialize<T>(shortOpt, longOpt, help, true, defaultVal, std::vector<T>(), action);
            return *this;
        }
        template <typename T>
        Option& create(std::string longOpt, std::string help,
                       T defaultVal, ACTION action = NONE) {
            _initialize<T>('\0', longOpt, help, true, defaultVal, std::vector<T>(), action);
            return *this;
        }
        template <typename T>
        Option& create(char shortOpt, std::string longOpt, std::string help) {
            _initialize<T>(shortOpt, longOpt, help, false, T(), std::vector<T>(), NONE);
            return *this;
        }
        template <typename T>
        Option& create(std::string longOpt, std::string help) {
            _initialize<T>('\0', longOpt, help, false, T(), std::vector<T>(), NONE);
            return *this;
        }
        Option(const Option&);
        Option& operator = (const Option& rhs);

        bool setValue(const std::string& value);
        void unsetValue();

        std::string getLongFlag() const { return _longOpt; }
        char getShortFlag() const { return _shortOpt; }
        ACTION getAction() const { return _action; }
        // bool isValid(std::string) const override;
        bool isValid() const;
        bool isValid(const std::string& s) const;
        std::string help() const override;
        std::string signature(int margin) const override;
        std::string getValue() const { return _value.getValue(); };
        template <typename T> T getValue() const {
            return _value.getValue<T>();
        }
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
            _templateType = ArgumentValue::templateToType<T>();
            _name = name;
            _help = help;
            _valueType = getValueType<T>();
            _minValues = minValues;
            if(maxValues < 1) throw std::invalid_argument("Can not have less than 1 maxValue for PositionalArgument");
            _maxValues = maxValues;
            return *this;
        }

        void addValue(const std::string& value);
        void unsetValues();

        size_t getMinArgs() const { return _minValues; }
        size_t getMaxArgs() const { return _maxValues; }
        bool isValid() const;
        bool isValid(const std::string& s) const { return Argument::isValid(s); }
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
    };

    class ArgumentParser {
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

        static bool _splitShortOption(std::string& arg, std::string& flag, std::string& value);
        static bool _splitLongOption(std::string arg, std::string& flag, std::string& value);
        static bool isFlag(std::string arg) {
            return !arg.empty() && arg[0] == '-';
        }
        bool _isOption(std::string arg) const;
        bool _parsePositionalArgs(int, int, char**);
        bool _parseShortOption(int&, int, char**, std::string);
        bool _parseLongOption(int&, int, char**, std::string);
        void _validatePositionalArgs() const;
        argparse::PositionalArgument* _nextArg(size_t&, bool);
        bool _doOptionAction(Option::ACTION, bool&) const;
        void _addOption(const Option& option);
    public:
        explicit ArgumentParser(std::string description = "", std::string programName = "", bool help = true) {
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
        void addOption(char shortOpt, std::string longOpt, std::string help) {
            _addOption(Option().create<T>(shortOpt, longOpt, help));
        }
        template <typename T>
        void addOption(std::string longOpt, std::string help) {
            _addOption(Option().create<T>('\0', longOpt, help, false));
        }
        template <typename T>
        void addOption(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal, Option::ACTION action = Option::ACTION::NONE) {
            _addOption(Option().create<T>(shortOpt, longOpt, help, defaultVal, action));
        }
        template <typename T>
        void addOption(std::string longOpt, std::string help,
                       T defaultVal, Option::ACTION action = Option::ACTION::NONE) {
            _addOption(Option().create<T>(longOpt, help, defaultVal, action));
        }
        template <typename T>
        void addOption(char shortOpt, std::string longOpt, std::string help,
                       T defaultVal, const std::vector<T>& choices) {
            _addOption(Option().create<T>(shortOpt, longOpt, help, defaultVal, choices));
        }
        template <typename T> void addOption(std::string longOpt, std::string help,
                                             T defaultVal, const std::vector<T>& choices) {
            _addOption(Option().create<T>(longOpt, help, defaultVal, choices));
        }
        void setVersion(std::string version, char shortOpt = 'v', std::string longOpt = "version");
        template <typename T>
        void addArgument(std::string name, std::string help, size_t minArgs = 1, size_t maxArgs = 1) {
            if(_args.find(name) != _args.end())
                throw std::runtime_error("'" + name + "' is already a positional argument");
            _args[name] = argparse::PositionalArgument().create<T>(name, help, minArgs, maxArgs);
            _argOrder.push_back(name);
        }
        void addArgument(std::string name, std::string help, size_t minArgs = 1, size_t maxArgs = 1) {
            addArgument<std::string>(name, help, minArgs, maxArgs);
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
        size_t nArgs(std::string argName) const;
        size_t nPositionalArgs() const { return _args.size(); }
        const PositionalArgument& getArgument(std::string name) {
            return _args.at(name);
        }
        static void splitOption(std::string arg, std::string& flag, std::string& value);
    };
}

#endif
