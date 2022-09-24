
#include <params.hpp>

std::string params::Argument::validArgNamePattern = "^([a-zA-Z][a-zA-Z0-9_\\-]+|[a-zA-Z0-9])$";
size_t params::Argument::maxLineLen = params::MAX_LINE_LEN;
size_t params::Argument::indendentLen = params::INDENT_LEN;

std::string params::Argument::typeToStr(params::Argument::TYPE type) {
    switch(type) {
        case STRING: return "std::string";
        case CHAR: return "char";
        case BOOL: return "bool";
        case INT: return "int";
        case FLOAT: return "float";
    }
}

//! Use params::VALID_ARG_NAME_REGEX to check if option name is valid
bool params::Argument::isValidName(std::string name) {
    return std::regex_match(name, std::regex(validArgNamePattern));
}

void params::Argument::_checkValidName() const {
    if(!isValidName(_name))
        throw std::invalid_argument("'" + _name + "' contains invalid characters!");
}

void params::Option::_checkOptFlags() const {
    if(_shortOpt.empty() && _longOpt.empty())
        throw std::invalid_argument("Long or short option must be specified!");
    if((!_shortOpt.empty() && _shortOpt[0] == '-') || (!_longOpt.empty() && _longOpt[0] == '-'))
        throw std::invalid_argument("Option flags can not begin with '-'!");
    if((!_shortOpt.empty() && !isValidName(_shortOpt)) || (!_longOpt.empty() && !isValidName(_longOpt)))
        throw std::invalid_argument("Long or short option flag contain invalid characters!");
}

params::Option::Option(std::string shortOpt, std::string longOpt, std::string help,
                       Option::TYPE valueType, std::string defaultVal,
                       Option::ACTION action) {
    _shortOpt = shortOpt;
    _longOpt = longOpt;
    _checkOptFlags();
    _name = _longOpt.empty() ? _shortOpt : _longOpt;
    _help = help;
    _isSet = false;
    _valueType = valueType;

    _action = action;
    if(_action != ACTION::NONE && !defaultVal.empty()) {
        if(_action == ACTION::STORE_TRUE) _value = "false";
        if(_action == ACTION::STORE_FALSE) _value = "true";
    }
    if(!isValid()) throw std::invalid_argument("Invalid option!");

    // validate default arg if necessary
    if (!defaultVal.empty() && !isValid(defaultVal))
        throw std::invalid_argument(defaultVal + " is an invalid value for TYPE!");
    _value = defaultVal;
    _defaultValue = defaultVal;
}

params::Argument::Argument() {
    _isSet = false;
    _valueType = TYPE::STRING;
}

params::PositionalArgument::PositionalArgument(std::string name, std::string help,
                                               size_t minValues, size_t maxValues,
                                               Argument::TYPE valueType) : Argument() {
    _name = name;
    _checkValidName();
    _help = help;
    _valueType = valueType;
    _minValues = minValues;
    if(maxValues < 1) throw std::invalid_argument("Can not have less than 1 maxValue for PositionalArgument");
    _maxValues = maxValues;
}

params::Argument::Argument(const params::Argument& rhs) {
    _name = rhs._name;
    _help = rhs._help;
    _valueType = rhs._valueType;
    _isSet = rhs._isSet;
}

params::Option::Option(const Option& rhs) : Argument(rhs) {
    _value = rhs._value;
    _shortOpt = rhs._shortOpt;
    _longOpt = rhs._longOpt;
    _defaultValue = rhs._defaultValue;
    _action = rhs._action;
}

params::Argument& params::Argument::operator = (const params::Argument& rhs) {
    _name = rhs._name;
    _help = rhs._help;
    _valueType = rhs._valueType;
    _isSet = rhs._isSet;
    return *this;
}

params::Option& params::Option::operator = (const params::Option& rhs) {
    Argument::operator=(rhs);
    _value = rhs._value;
    _shortOpt = rhs._shortOpt;
    _longOpt = rhs._longOpt;
    _defaultValue = rhs._defaultValue;
    _action = rhs._action;
    return *this;
}

bool params::Option::isValid(std::string value) const {
    if(_valueType != TYPE::BOOL && _action != ACTION::NONE)
        return false; // Has to be NONE unless _valueType is BOOL.
    return Argument::isValid(value);
}

//! Check if \p value can be converted into \p params::Argument::valueType.
bool params::Argument::isValid(std::string value) const {
    if(_valueType == TYPE::STRING) return true; // it's already a string so it's valid.
    if(_valueType == TYPE::CHAR) return value.size() == 1;
    if(_valueType == TYPE::BOOL) {
        return std::regex_match(value, std::regex("^(true|false|0|1)$", std::regex_constants::icase));
    }
    if(_valueType == TYPE::INT) {
        return std::regex_match(value, std::regex("^-?[0-9]+$"));
    }
    if(_valueType == TYPE::FLOAT) {
        return std::regex_match(value, std::regex("^-?[0-9]+(\\.[0-9]*)?$"));
    }
    return false;
}

bool params::Option::isValid() const {
    if(_valueType != TYPE::BOOL && _action != ACTION::NONE) return false;
    if(_value.empty()) return true;
    return isValid(_value);
}

bool params::PositionalArgument::isValid () const {
    if(getArgCount() < _minValues || getArgCount() > _maxValues)
        return false;
    for(const auto& value: _values) {
        if (!Argument::isValid(value))
            return false;
    }
    return true;
}

std::string params::PositionalArgument::invalidReason() const {
    if(getArgCount() < _minValues)
        return "Not enough arguments given!";
    if(getArgCount() > _maxValues)
        return "Too many arguments given!";
    for(const auto& value: _values) {
        if (!Argument::isValid(value))
            return "No viable conversion of " + value + " to " + typeToStr(_valueType);
    }
    return "";
}

bool params::Option::setValue(std::string value) {
    if(_valueType == TYPE::BOOL && _action != ACTION::NONE){
        if(!value.empty()) return false;
        if(_action == ACTION::STORE_TRUE) _value = "true";
        if(_action == ACTION::STORE_FALSE) _value = "false";
        if(_action == ACTION::HELP) _value = "true";
        if(_action == ACTION::VERSION) _value = "true";
    }
    else {
        _value = value;
    }
    if(isValid()) {
        _isSet = true;
        return true;
    }
    return false;
}

void params::Option::unsetValue() {
    _isSet = false;
    _value = _defaultValue;
}

bool params::PositionalArgument::addValue(std::string value) {
    _values.push_back(value);
    if(Argument::isValid(value)) {
        _isSet = true;
        return true;
    }
    return false;
}

void params::PositionalArgument::unsetValues() {
    _isSet = false;
    _values.clear();
}

std::string params::Option::parseOption(std::string arg) {
    std::string ret = arg;
    size_t len = std::min(size_t(2), arg.size());
    for(size_t i = 0; i < len; i++) {
        if(arg[i] == '-') ret.erase(0, 1);
    }
    return ret;
}

std::string params::Argument::help() const {
    return _help;
}

std::string params::escapeEscapeCharacters(const std::string& s) {
    std::string ret;
    for(auto c: s) {
        switch(c) {
            case '\'': ret += c; break;
            case '\?': ret += "\\?"; break;
            case '\\': ret += "\\\\"; break;
            case '\a': ret += "\\a"; break;
            case '\b': ret += "\\b"; break;
            case '\f': ret += "\\f"; break;
            case '\n': ret += "\\n"; break;
            case '\r': ret += "\\r"; break;
            case '\t': ret += "\\t"; break;
            case '\v': ret += "\\v"; break;
            default: ret += c;
        }
    }
    return ret;
}

std::string params::Option::help() const {
    std::string ret = Argument::help();
    // if(_valueType == TYPE::BOOL && _action != ACTION::NONE)
    if(!_defaultValue.empty() && _action == ACTION::NONE) {
        ret += " '" + escapeEscapeCharacters(_defaultValue) + "' is the default.";
    }
    return ret;
}

std::string params::Argument::signature(std::string ret, int margin) const {
    size_t spacesBeforeHelp = indendentLen > ret.size() ? indendentLen + margin - ret.size() : 0;
    if(spacesBeforeHelp <= 0) {
        ret += "\n";
        ret = std::string(margin, ' ') + ret + multiLineString(help(), margin, true);
    } else {
        ret += std::string(spacesBeforeHelp - margin, ' ');
        ret += help();
        ret = multiLineString(ret, margin);
    }

    return ret;
}

std::string params::Option::signature(int margin) const {
    std::string ret;
    std::string name = (_action == ACTION::NONE ? _name : "");
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    if(!_shortOpt.empty()) {
        ret += "-" + _shortOpt + (name.empty() ? "" : ' ' + name);
        if(!_longOpt.empty()) ret += ", ";
    }
    if(!_longOpt.empty())
        ret += "--" + _longOpt + (name.empty() ? "" : ' ' + name);

    return Argument::signature(ret, margin);
}

std::string params::PositionalArgument::signature(int margin) const {
    return Argument::signature(_name, margin);
}

std::string params::Argument::multiLineString(std::string addStr, size_t margin, bool indentFirstLine) {
    return params::multiLineString(addStr, margin, maxLineLen, indendentLen, indentFirstLine);
}

void params::Params::addOption(std::string shortOpt, std::string longOpt, std::string help,
                               params::Option::TYPE valueType, std::string defaultVal,
                               params::Option::ACTION action) {
    params::Option option(shortOpt, longOpt, help, valueType, defaultVal, action);
    std::string name = option.getName();
    _options[name] = option;
    _optionOrder.push_back(name);

    // add option to _optionKeys map
    std::string flags[2] = {shortOpt, longOpt};
    for(auto & flag : flags) {
        if (!flag.empty()) {
            if(_optionKeys.find(flag) != _optionKeys.end())
                throw std::runtime_error("'" + flag + "' already exists as an option!");
            _optionKeys[flag] = name;
        }
    }
}

//! Set program version and add version option flag;
void params::Params::setVersion(std::string version, std::string shortOpt, std::string longOpt) {
    _version = version;
    addOption(shortOpt, longOpt, "Print version and exit", Option::TYPE::BOOL, "false", Option::ACTION::VERSION);
}

void params::Params::addArgument(std::string name, std::string help,
                                 size_t minArgs, size_t maxArgs,
                                 params::Option::TYPE valueType) {
    if(_args.find(name) != _args.end())
        throw std::runtime_error("'" + name + "' is already a positional argument");
    _args[name] = params::PositionalArgument(name, help, minArgs, maxArgs, valueType);
    _argOrder.push_back(name);
}

/**
 * Determine whether \p arg is an option in _options
 * @param arg A potential option flag (with dash included).
 * @return true if \p arg is an existing option.
 */
bool params::Params::_isOption(std::string arg) const {
    if(!isFlag(arg)) return false;
    std::string key = Option::parseOption(arg);
    return _optionKeys.find(key) != _optionKeys.end();
}

void params::Params::printHelp() const {
    std::cout << "Usage: " << signature() << "\n\n" << params::Option::multiLineString(_description, 0) << "\n";

    //options
    if(!_options.empty()) {
        std::cout << "\nOptions:";
        for(const auto& option: _optionOrder)
            std::cout << std::endl << _options.at(option).signature(_helpMargin);
        std::cout << std::endl;
    }

    // positional arguments
    if(!_args.empty()) {
        std::cout << "\nPositional arguments:";
        for (const auto &arg: _argOrder)
            std::cout << std::endl << _args.at(arg).signature(_helpMargin);
    }
    std::cout << std::endl;
}

std::string params::Params::signature() const {
    std::string ret = _programName;
    if(!_options.empty()) ret += " [options]";
    if(!_args.empty()) {
        for(const auto& arg: _argOrder) {
            ret += " ";
            if(_args.at(arg).getMinArgs() == 0)
                ret += "[";
            ret += "<" + _args.at(arg).getName() + ">";
            if(_args.at(arg).getMaxArgs() > 1)
                ret += " [...]";
            else if(_args.at(arg).getMinArgs() == 0)
                ret += "]";
        }
    }
    return ret;
}

void params::Params::usage(std::ostream& out) const {
    out << signature();
}

void params::Params::printVersion() const {
    std::cout << _programName << " " << _version << std::endl;
}

params::PositionalArgument* params::Params::_nextArg(size_t& currentArgIndex, bool endOfArgv) {
    if(currentArgIndex > nPositionalArgs()) return nullptr;
    if(endOfArgv) {
        currentArgIndex = _argOrder.size() - 1;
    }
    else {
        size_t remaining = nArgs(_argOrder[currentArgIndex]) - _args.at(_argOrder[currentArgIndex]).getMaxArgs();
        if (remaining == 0) currentArgIndex++;
    }
    return &_args.at(_argOrder[currentArgIndex]);
}

bool params::Params::_parsePositionalArgs(int i, int argc, char** argv) {
    _validatePositionalArgs();
    params::PositionalArgument* currentArg = nullptr;
    size_t currentArgIndex = 0;
    for(; i < argc ; i++) {
        currentArg = _nextArg(currentArgIndex, i + 1 >= argc);
        if(!currentArg) {
            std::cerr << "ERROR: Too many positional arguments!\n";
            usage();
            return false;
        }
        if(_singleDashBehavior == ERROR && isFlag(argv[i])) {
            usage();
            return false;
        }
        currentArg->addValue(std::string(argv[i]));
    }
    return checkPositionalArgs();
}

//! Check that the number of required positional args were given by the user on the command line.
bool params::Params::checkPositionalArgs() const {
    bool allGood = true;
    for(const auto& arg: _args) {
        if(!arg.second.isValid()) {
            if(allGood) usage();
            allGood = false;
            std::cerr << "\nInvalid argument for '" <<  arg.second.getName() << "': "
                      << arg.second.invalidReason() << std::endl;
        }
    }
    return allGood;
}

//! Check that the structure of the positional arguments set by the developer is valid.
void params::Params::_validatePositionalArgs() const {
    int infArgCount = 0;
    size_t nArgs = _argOrder.size();
    for(size_t i = 0; i < nArgs; i++) {
        if(_args.at(_argOrder[i]).getMaxArgs() == std::string::npos)
            infArgCount++;
        if(i < nArgs - 1 && _args.at(_argOrder[i]).getMinArgs() == 0)
            throw std::runtime_error("Only the last positional argument can have a minimum of 0 values!");
        if(_args.at(_argOrder[i]).getMinArgs() == std::string::npos)
            throw std::runtime_error("Minimum number of args must be finite!");
    }
    if(infArgCount > 1)
        throw std::runtime_error("Can not have more than 1 positional argument with infinite values.");
}

bool params::Params::_doOptionAction(Option::ACTION action, bool& returnVal) const {
    HELP_VERSION_BEHAVIOR behavior;
    if(action == Option::HELP) {
        printHelp();
        behavior = _helpBehavior;
    }
    if(action == Option::VERSION) {
        printVersion();
        behavior = _versionBehavior;
    }

    returnVal = true;
    switch(behavior) {
        case CONTINUE: return false;
        case RETURN_TRUE:
            returnVal = true;
            return true;
        case RETURN_FALSE:
            returnVal = false;
            return true;
        case EXIT_0: exit(0);
        case EXIT_1: exit(1);
    }
}

bool params::Params::parseArgs(int argc, char** argv)
{
    clearArgs();
    _validatePositionalArgs();
    if(_programName.empty())
        _programName = std::filesystem::path(std::string(argv[0])).filename();
    for(int i = 1; i < argc; i++)
    {
        if(isFlag(std::string(argv[i])))
        {
            std::string option = std::string(argv[i]);
            if(!_isOption(option)) {
                std::cerr << "ERROR: '" << option << "' is an unknown option\n";
                return false;
            }

            std::string value;
            std::string key = _optionKeys.at(Option::parseOption(option));
            if(_options.at(key).getAction() == Option::HELP || _options.at(key).getAction() == Option::VERSION) {
                bool returnVal;
                if(_doOptionAction(_options.at(key).getAction(), returnVal))
                    return returnVal;
            }

            if(_options.at(key).getType() == Option::TYPE::BOOL &&
               _options.at(key).getAction() != Option::ACTION::NONE) {
                value = "";
            }
            else {
                if(i + 1 >= argc || !isFlag(std::string(argv[++i]))) {
                    value = std::string(argv[i]);
                } else {
                    std::cerr << "ERROR: Missing argument for '" << option << "'\n";
                    return false;
                }
            }
            if(!_options.at(key).setValue(value)) {
                std::cerr << "ERROR: '" << value << "' is an invalid argument for '" << option << "'\n";
                return false;
            }
        }
        else if (strcmp(argv[i], "-") == 0) {
            switch (_singleDashBehavior) {
                case START_POSITIONAL:
                    return _parsePositionalArgs(i, argc, argv);
                case ERROR:
                    usage();
                    return false;
            }
        }
        else { // we are in positional arguments
            return _parsePositionalArgs(i, argc, argv);
        }
    }
    return true;
}

void params::Argument::toType(const std::string& value, char& dest) const {
    if(_valueType != CHAR) throw std::runtime_error("Converting " + typeToStr(_valueType) + " is undefined!");
    dest = value[0];
}

void params::Argument::toType(const std::string& value, bool& dest) const {
    if(_valueType != BOOL) throw std::runtime_error("Converting " + typeToStr(_valueType) + " is undefined!");
    std::string temp = value;
    std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
    if(temp == "true") dest = true;
    else if(temp == "false") dest = false;
    else dest = bool(std::stoi(value));
}

void params::Argument::toType(const std::string& value, int& dest) const {
    if(_valueType != INT) throw std::runtime_error("Converting " + typeToStr(_valueType) + " is undefined!");
    dest = std::stoi(value);
}

void params::Argument::toType(const std::string& value, float& dest) const {
    if(_valueType != FLOAT) throw std::runtime_error("Converting " + typeToStr(_valueType) + " is undefined!");
    dest = std::stof(value);
}

std::vector<std::string> params::Params::getArgumentValues(std::string argName) const {
    return _args.at(argName).getValues();
}

/**
 * Return a single argument for PositionalArguments that can only have a single value.
 * @return The argument value
 * @throws std::runtime_error If the PositionalArgument can have more than 1 value.
 * @throws std::out_of_range If no value was given for PositionalArgument on the command line.
 */
std::string params::PositionalArgument::getValue() const {
    if (_minValues > 1 || _maxValues > 1)
        throw std::runtime_error("Calls to this function are invalid when there are more than 1 values for argument!");
    if (getArgCount() != 1)
        throw std::out_of_range("'" + _name + "' is empty!");
    return _values[0];
}

//! Number of args given for \p argName on the command line.
size_t params::Params::nArgs(std::string argName) const {
    return _args.at(argName).getArgCount();
}

std::string params::Params::getOptionValue(std::string option) const {
    return _options.at(_optionKeys.at(option)).getValue();
}

//! Return options to their default state, and clear positional arguments
void params::Params::clearArgs() {
    for (auto it = _options.begin(); it != _options.end(); ++it)
        it->second.unsetValue();
    for (auto it = _args.begin(); it != _args.end(); ++it)
        it->second.unsetValues();
}

std::vector<std::string> params::PositionalArgument::getValues() const {
    return _values;
}

bool params::newWord(char c) {
    return c == ' ';
}

std::string params::multiLineString(std::string addStr, size_t margin,
                                    size_t maxLineLen, size_t indentLen,
                                    bool indentFirstLine)
{
    if(maxLineLen - margin < 20)
        throw std::runtime_error("Can't have text on line less than 20 characters!");

    // Fix white spaces in addStr
    std::replace(addStr.begin(), addStr.end(), '\t', ' ');
    std::replace(addStr.begin(), addStr.end(), '\n', ' ');

    std::vector<std::string> lines;
    size_t lineLen = 0;
    size_t addStrLen = addStr.size();
    std::string word;
    bool beginingOfLine = true;
    for(size_t i = 0; i < addStrLen;)
    {
        if(i > 0 && !word.empty()) i++;
        if(indentFirstLine) lines.emplace_back(margin + indentLen, ' ');
        else lines.emplace_back((lines.empty() ? margin : margin + indentLen), ' ');

        lines.back() += word;
        beginingOfLine = word.empty();
        lineLen = lines.back().size();
        while(i < addStrLen && lineLen < maxLineLen) {
            word = "";
            while(i < addStrLen && !newWord(addStr[i]))  {
                word += addStr[i++];
            }
            if(lines.back().size() + word.size() + 1 <= maxLineLen) {
                i++;
                if(beginingOfLine) beginingOfLine = false;
                else lines.back() += ' ';
                lines.back() += word;
                word = "";
            } else {
                break;
            }
            lineLen = lines.back().size();
        }
    }
    if(!word.empty()) {
        lines.emplace_back(std::string(margin + indentLen, ' ') + word);
    }

    std::string ret;
    for(size_t i = 0; i < lines.size(); i++) {
        if(i > 0) ret += '\n';
        ret += lines[i];
    }
    return ret;
}
