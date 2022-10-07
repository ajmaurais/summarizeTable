
#include <params.hpp>

std::string params::Option::validLongOptionPattern = "[a-zA-Z0-9][a-zA-Z0-9_\\-\\.:]+";
std::string params::Option::validShortOptionPattern = "[a-zA-Z0-9]";
size_t params::Argument::maxLineLen = params::MAX_LINE_LEN;
size_t params::Argument::indendentLen = params::INDENT_LEN;

std::string params::ArgumentValue::valueTypeToStr(params::ArgumentValue::VALUE_TYPE type) {
    switch(type) {
        case STRING: return "std::string";
        case CHAR: return "char";
        case DOUBLE: return "double";
        case LONG: return "long";
        case SIZE_T: return "size_t";
        case BOOL: return "bool";
        case INT: return "int";
        case FLOAT: return "float";
        default: throw std::runtime_error("Variant index undefined!");
    }
}

std::string params::ArgumentValue::valueTypeStr() const {
    return valueTypeToStr(VALUE_TYPE(_value.index()));
}

params::ArgumentValue::ArgumentValue(VALUE_TYPE type) {
    switch(type) {
        case VALUE_TYPE::BOOL: setValue<bool>(); return;
        case VALUE_TYPE::INT: setValue<int>(); return;
        case VALUE_TYPE::LONG: setValue<long>(); return;
        case VALUE_TYPE::SIZE_T: setValue<size_t>(); return;
        case VALUE_TYPE::FLOAT: setValue<float>(); return;
        case VALUE_TYPE::DOUBLE: setValue<double>(); return;
        case VALUE_TYPE::CHAR: setValue<char>(); return;
        case VALUE_TYPE::STRING: setValue<std::string>(); return;
    }
}

void params::ArgumentValue::setValue(std::string value) {
    if(!isValid(VALUE_TYPE(_value.index()), value))
        throw std::invalid_argument("'" + value + "' Can not be converted to " +
                                    valueTypeToStr(VALUE_TYPE(_value.index())));
    switch(_value.index()) {
        case VALUE_TYPE::BOOL: _setValue<bool>(value); return;
        case VALUE_TYPE::INT: _setValue<int>(value); return;
        case VALUE_TYPE::LONG: _setValue<long>(value); return;
        case VALUE_TYPE::SIZE_T: _setValue<size_t>(value); return;
        case VALUE_TYPE::FLOAT: _setValue<float>(value); return;
        case VALUE_TYPE::DOUBLE: _setValue<double>(value); return;
        case VALUE_TYPE::CHAR: _setValue<char>(value); return;
        case VALUE_TYPE::STRING: setValue<std::string>(value); return;
        default: throw std::runtime_error("Variant index undefined!");
    }
}

std::string params::ArgumentValue::str() const {
    switch(_value.index()) {
        case BOOL: return (std::get<bool>(_value) ? "true" : "false");
        case INT: return std::to_string(std::get<int>(_value));
        case LONG: return std::to_string(std::get<long>(_value));
        case SIZE_T: return std::to_string(std::get<size_t>(_value));
        case FLOAT: return std::to_string(std::get<float>(_value));
        case DOUBLE: return std::to_string(std::get<double>(_value));
        case CHAR: return std::string(1, std::get<char>(_value));
        case STRING: return std::get<std::string>(_value);
        default: throw std::runtime_error("Variant index undefined!");
    }
}

void params::ArgumentValue::toType(bool& lhs, const std::string& rhs) {
    std::string temp = rhs;
    std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
    if(temp == "true") lhs = true;
    else if(temp == "false") lhs = false;
    else lhs = bool(std::stoi(rhs));
}

void params::ArgumentValue::toType(int& lhs, const std::string& rhs) {
    lhs = std::stoi(rhs);
}

void params::ArgumentValue::toType(long& lhs, const std::string& rhs) {
    lhs = std::stol(rhs);
}

void params::ArgumentValue::toType(size_t& lhs, const std::string& rhs) {
    lhs = std::stoul(rhs);
}

void params::ArgumentValue::toType(float& lhs, const std::string& rhs) {
    lhs = std::stof(rhs);
}

void params::ArgumentValue::toType(double& lhs, const std::string& rhs) {
    lhs = std::stod(rhs);
}

void params::ArgumentValue::toType(char& lhs, const std::string& rhs) {
    lhs = rhs[0];
}

std::string params::Argument::typeToStr(params::Argument::TYPE type) {
    switch(type) {
        case STRING: return "std::string";
        case CHAR: return "char";
        case BOOL: return "bool";
        case INT: return "int";
        case FLOAT: return "float";
    }
}

bool params::Argument::isNumeric(params::Argument::TYPE type) {
    switch(type) {
        case STRING:
        case CHAR:
            return false;
        case BOOL:
        case INT:
        case FLOAT:
            return true;
    }
}

void params::Option::_checkOptFlags() const {
    if(_shortOpt == '\0' && _longOpt.empty())
        throw std::invalid_argument("Long or short option must be specified!");
    if((_shortOpt == '-') || (!_longOpt.empty() && _longOpt[0] == '-'))
        throw std::invalid_argument("Option flags can not begin with '-'!");
    if(_shortOpt != '\0' && !std::regex_match(std::string(1, _shortOpt), std::regex(validShortOptionPattern)))
        throw std::invalid_argument("Short option flag contains invalid characters!");
    if(!_longOpt.empty() && !std::regex_match(_longOpt, std::regex(validLongOptionPattern)))
        throw std::invalid_argument("Long option flag contains invalid characters!");
}

params::Argument::Argument() {
    _isSet = false;
    _valueType = TYPE::STRING;
}

params::Argument::Argument(const params::Argument& rhs) {
    _name = rhs._name;
    _help = rhs._help;
    _valueType = rhs._valueType;
    _templateType = rhs._templateType;
    _isSet = rhs._isSet;
}

params::Option::Option(const Option& rhs) : Argument(rhs) {
    _value = rhs._value;
    _shortOpt = rhs._shortOpt;
    _longOpt = rhs._longOpt;
    _defaultValue = rhs._defaultValue;
    _choices = rhs._choices;
    _action = rhs._action;
}

params::Argument& params::Argument::operator = (const params::Argument& rhs) = default;

params::Option& params::Option::operator = (const params::Option& rhs) = default;

//! Check if \p value can be converted into \p params::Argument::valueType.
bool params::ArgumentValue::isValid(params::ArgumentValue::VALUE_TYPE type, const std::string& value) {
    switch(type) {
        case VALUE_TYPE::STRING: return true; // it's already a string so it's valid.
        case VALUE_TYPE::CHAR: return value.size() == 1;
        case VALUE_TYPE::BOOL:
            return std::regex_match(value, std::regex("^(true|false|0|1)$", std::regex_constants::icase));
        case VALUE_TYPE::INT:
        case VALUE_TYPE::LONG:
        case VALUE_TYPE::SIZE_T:
            return std::regex_match(value, std::regex("^-?[0-9]+$"));
        case VALUE_TYPE::FLOAT:
        case VALUE_TYPE::DOUBLE:
            return std::regex_match(value, std::regex("^-?[0-9]+(\\.[0-9]*)?$"));
    }
}

bool params::ArgumentValue::isValid(const std::string& s) const{
    return isValid(VALUE_TYPE(_value.index()), s);
}

bool params::Argument::isValid(std::string s) const{
   switch(_valueType) {
       case TYPE::STRING: return ArgumentValue::isValid(ArgumentValue::VALUE_TYPE::STRING, s);
       case TYPE::CHAR: return ArgumentValue::isValid(ArgumentValue::VALUE_TYPE::CHAR, s);
       case TYPE::BOOL: return ArgumentValue::isValid(ArgumentValue::VALUE_TYPE::BOOL, s);
       case TYPE::INT: return ArgumentValue::isValid(ArgumentValue::VALUE_TYPE::INT, s);
       case TYPE::FLOAT: return ArgumentValue::isValid(ArgumentValue::VALUE_TYPE::FLOAT, s);
   }
}

//! Return true if after option is set, option is valid.
bool params::Option::isValid() const {
    if(_valueType != TYPE::BOOL && _action != ACTION::NONE)
        return false;
    if(!_choices.empty() && _choices.find(_value) == _choices.end())
        return false;
    if(!_defaultValue.isSet() && !_value.isSet())
        return false;
    return true;
}

/**
 * Return true if \p s is a valid value for option.
 * Specifically, check if \p s can be converted into valid option value,
 * and check if \p s is a valid choice for option.
 */
bool params::Option::isValid(const std::string &s) const {
    if(!Argument::isValid(s))
        return false;
    ArgumentValue find_s(_templateType);
    find_s.setValue(s);
    if(!_choices.empty() && _choices.find(find_s) == _choices.end())
        return false;
    return true;
}

bool params::PositionalArgument::isValid () const {
    if(getArgCount() < _minValues || getArgCount() > _maxValues)
        return false;
    return true;
}

std::string params::PositionalArgument::invalidReason() const {
    if(getArgCount() < _minValues)
        return "Not enough arguments given!";
    if(getArgCount() > _maxValues)
        return "Too many arguments given!";
    return "";
}

bool params::Option::setValue(const std::string& value) {
    unsetValue();
    if(_valueType == TYPE::BOOL && _action != ACTION::NONE){
        if(!value.empty()) return false;
        if(_action == ACTION::STORE_TRUE) _value.setValue<bool>(true);
        if(_action == ACTION::STORE_FALSE) _value.setValue<bool>(false);
        if(_action == ACTION::HELP) _value.setValue<bool>(true);
        if(_action == ACTION::VERSION) _value.setValue<bool>(true);
    }
    else {
        if(!Argument::isValid(value)) return false;
        _value.setValue(value);
    }
    _isSet = true;
    return isValid();
}

void params::Option::unsetValue() {
    _isSet = false;
    _value = _defaultValue;
}

void params::PositionalArgument::addValue(const std::string& value) {
    _values.emplace_back(_templateType);
    if(_values.back().isValid(value)) {
        _isSet = true;
    }
    _values.back().setValue(value);
}

void params::PositionalArgument::unsetValues() {
    _isSet = false;
    _values.clear();
}

std::string params::unquote(std::string s) {
    std::smatch match;
    if(!std::regex_match(s, match, std::regex(R"(\"(.*)\"|'(.*)')")))
        return s;
    return (match.begin() + 1)->matched ? match[1] : match[2];
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
    if(_defaultValue.isSet() && _action == ACTION::NONE) {
        ret += " '" + escapeEscapeCharacters(_defaultValue.str()) + "' is the default.";
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
    std::string name;
    if(_action != ACTION::NONE) {
        name = "";
    }
    else if(!_choices.empty()) {
        std::string quote = isNumeric(_valueType) ? "" : "'";
        // char quote = isNumeric(_valueType) ? '\0' : '\'';
        name = "{" + quote;
        int i = 0;
        for(const auto& choice: _choices) {
            if(i > 0) name += ", " + quote;
            name += choice.str() + quote;
            i++;
        }
        name += "}";
    }
    else {
        name = _name;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    }
    if(_shortOpt != '\0') {
        ret += "-" + std::string(1, _shortOpt) + (name.empty() ? "" : ' ' + name);
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

void params::Params::_addOption(const params::Option &option) {
    std::string name = option.getName();
    _options[name] = option;
    _optionOrder.push_back(name);

    // add option to _optionKeys map
    std::string flags[2] = {(option.getShortFlag() == '\0' ? "" : std::string(1, option.getShortFlag())), option.getLongFlag()};
    for(auto & flag : flags) {
        if (!flag.empty()) {
            if(_optionKeys.find(flag) != _optionKeys.end())
                throw std::runtime_error("'" + flag + "' already exists as an option!");
            _optionKeys[flag] = name;
        }
    }
}

//! Set program version and add version option flag;
void params::Params::setVersion(std::string version, char shortOpt, std::string longOpt) {
    _version = version;
    addOption<bool>(shortOpt, longOpt, "Print version and exit", false, Option::ACTION::VERSION);
}

/**
 * Determine whether \p arg is an option in _options
 * @param arg A potential option flag (with dash included).
 * @return true if \p arg is an existing option.
 */
bool params::Params::_isOption(std::string arg) const {
    if(!isFlag(arg)) return false;
    std::string key, value;
    splitOption(arg, key, value);
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
    out << signature() << std::endl;
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

void params::Params::splitOption(std::string s, std::string& flag, std::string& value) {
    bool isLongOption = s.substr(0, std::min(size_t(2), s.size())) == "--";

    if(isLongOption) _splitLongOption(s, flag, value);
    else _splitShortOption(s, flag, value);
}

bool params::Params::_splitLongOption(std::string s, std::string& flag, std::string& value)
{
    flag.clear();
    value.clear();
    std::smatch match;
    std::regex pattern("--(" + Option::validLongOptionPattern + ")(=(.+))?");
    if(!std::regex_match(s, match, pattern)) {
        return false;
    }
    flag = match[1];
    value = unquote(match[3]);
    return true;
}

bool params::Params::_splitShortOption(std::string& s, std::string& flag, std::string& value)
{
    flag.clear();
    value.clear();
    if(s.empty()) return false;

    if(s[0] == '-') s.erase(0, 1);
    if(s.size() == 0) { // special case where flag is '-'
        flag = "-";
        return true;
    }
    flag = s[0];
    value = s.substr(1);
    s.clear();
    value = unquote(value);
    return true;
}

bool params::Params::_parseShortOption(int& i, int argc, char** argv, std::string option)
{
    std::string flag, value;
    while(Params::_splitShortOption(option, flag, value))
    {
        // avoid std::out_of_range when looking up option key.
        if(_optionKeys.find(flag) == _optionKeys.end()) {
            std::cerr << "ERROR: '" << option << "' is an unknown option\n";
            return false;
        }

        // Look up key associated with Option for flag
        std::string key = _optionKeys.at(flag);

        // Check that the option associated with the key has a short option matching flag
        if(_options.at(key).getShortFlag() != flag[0]) {
            std::cerr << "ERROR: '" << option << "' is an unknown option\n";
            return false;
        }

        // Handle special Option actions.
        if (_options.at(key).getAction() == Option::HELP || _options.at(key).getAction() == Option::VERSION) {
            bool returnVal;
            if (_doOptionAction(_options.at(key).getAction(), returnVal))
                return returnVal;
        }

        if (_options.at(key).getType() == Option::TYPE::BOOL &&
            _options.at(key).getAction() != Option::ACTION::NONE)
        {
            option = value + option;
            value = "";
        } else {
            if (value.empty()) {
                if (i + 1 <= argc && !isFlag(std::string(argv[++i]))) {
                    value = unquote(std::string(argv[i]));
                } else {
                    std::cerr << "ERROR: Missing argument for '" << option << "'\n";
                    return false;
                }
            }
        }
        if (!_options.at(key).setValue(value)) {
            std::cerr << "ERROR: '" << value << "' is an invalid argument for '" << flag << "'\n";
            return false;
        }
        if(option.empty()) break;
    }
    return true;
}

// std::string params::Option::validLongOptionPattern = "([a-zA-Z0-9][a-zA-Z0-9_\\-\\.:]*)(=(.+))?";
bool params::Params::_parseLongOption(int& i, int argc, char** argv, std::string option)
{
    std::string flag, value;
    if(!_splitLongOption(option, flag, value)) {
        std::cerr << "ERROR: Can not parse long option: '" << option << "'\n";
        return false;
    }

    // avoid std::out_of_range when looking up option key.
    if(_optionKeys.find(flag) == _optionKeys.end()) {
        std::cerr << "ERROR: '" << option << "' is an unknown option\n";
        return false;
    }

    // Look up key associated with Option for flag
    std::string key = _optionKeys.at(flag);

    // Check that the option associated with the key has a short option matching flag
    if(_options.at(key).getLongFlag() != flag) {
        std::cerr << "ERROR: '" << option << "' is an unknown option\n";
        return false;
    }

    // Handle special Option actions.
    if (_options.at(key).getAction() == Option::HELP || _options.at(key).getAction() == Option::VERSION) {
        bool returnVal;
        if (_doOptionAction(_options.at(key).getAction(), returnVal))
            return returnVal;
    }

    if (_options.at(key).getType() == Option::TYPE::BOOL &&
        _options.at(key).getAction() != Option::ACTION::NONE)
    {
        option = value + option;
        value = "";
    } else {
        if (value.empty()) {
            if (i + 1 <= argc && !isFlag(std::string(argv[++i]))) {
                value = unquote(std::string(argv[i]));
            } else {
                std::cerr << "ERROR: Missing argument for '" << option << "'\n";
                return false;
            }
        }
    }
    if (!_options.at(key).setValue(value)) {
        std::cerr << "ERROR: '" << value << "' is an invalid argument for '" << flag << "'\n";
        return false;
    }
    return true;
}

bool params::Params::parseArgs(int argc, char** argv)
{
    clearArgs();
    _validatePositionalArgs();
    if(_programName.empty())
        _programName = std::filesystem::path(std::string(argv[0])).filename();
    for(int i = 1; i < argc; i++)
    {
        if(isFlag(std::string(argv[i]))) {
            std::string option = std::string(argv[i]);
            if (!_isOption(option)) {
                if (option == "-") {
                    switch (_singleDashBehavior) {
                        case START_POSITIONAL:
                            return _parsePositionalArgs(i, argc, argv);
                        case ERROR:
                            usage();
                            return false;
                    }
                }
                std::cerr << "ERROR: '" << option << "' is an unknown option\n";
                return false;
            }
            bool isLongOption = option.substr(0, std::min(size_t(2), option.size())) == "--";
            bool success = isLongOption ? _parseLongOption(i, argc, argv, option) :
                                          _parseShortOption(i, argc, argv, option);
            if(!success) return false;
        }
        else { // we are in positional arguments
            return _parsePositionalArgs(i, argc, argv);
        }
    }
    return true;
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
    return _values[0].getValue();
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
