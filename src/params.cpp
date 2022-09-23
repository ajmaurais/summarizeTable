
#include <params.hpp>

std::string params::Argument::validArgNamePattern = "^([a-zA-Z][a-zA-Z0-9_\\-]+|[a-zA-Z0-9])$";
size_t params::Argument::maxLineLen = params::MAX_LINE_LEN;
size_t params::Argument::indendentLen = params::INDENT_LEN;

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

params::PositionalArgument::PositionalArgument(std::string name, std::string help, Argument::TYPE valueType) : Argument() {
    _name = name;
    _checkValidName();
    _help = help;
    _valueType = valueType;
}

params::Argument::Argument(const params::Argument& rhs) {
    _name = rhs._name;
    _help = rhs._help;
    _value = rhs._value;
    _valueType = rhs._valueType;
    _isSet = rhs._isSet;
}

params::Option::Option(const Option& rhs) : Argument(rhs) {
    _shortOpt = rhs._shortOpt;
    _longOpt = rhs._longOpt;
    _defaultValue = rhs._defaultValue;
    _action = rhs._action;
}

params::Argument& params::Argument::operator = (const params::Argument& rhs) {
    _name = rhs._name;
    _help = rhs._help;
    _value = rhs._value;
    _valueType = rhs._valueType;
    _isSet = rhs._isSet;
    return *this;
}

params::Option& params::Option::operator = (const params::Option& rhs) {
    Argument::operator=(rhs);
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
    if(!_isSet) return false;
    return Argument::isValid(_value);
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

bool params::PositionalArgument::setValue(std::string value) {
    _value = value;
    if(isValid()) {
        _isSet = true;
        return true;
    }
    return false;
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

//! Set program version and add version option.
void params::Params::setVersion(std::string version, std::string shortOpt, std::string longOpt) {
    _version = version;
    addOption(shortOpt, longOpt, "Print version and exit", Option::TYPE::BOOL, "false", Option::ACTION::VERSION);
}

void params::Params::addArgument(std::string name, std::string help, params::Option::TYPE valueType) {
    if(_args.find(name) != _args.end())
        throw std::runtime_error("'" + name + "' is already a positional argument");
    _args[name] = params::PositionalArgument(name, help, valueType);
    _argOrder.push_back(name);
}

/**
 * Determine whether \p arg is an option in _options
 * @param arg A potential option flag (with dash included).
 * @return true if \p arg is an existing option.
 */
bool params::Params::isOption(std::string arg) const {
    if(!isFlag(arg)) return false;
    std::string key = Option::parseOption(arg);
    return _optionKeys.find(key) != _optionKeys.end();
}

void params::Params::printHelp() const {
    size_t margin = 2;
    std::cout << "Usage: " << signature() << "\n\n" << params::Option::multiLineString(_description, 0) << "\n";

    //options
    if(!_options.empty()) {
        std::cout << "\nOptions:";
        for(const auto& option: _optionOrder)
            std::cout << std::endl << _options.at(option).signature(_helpMargin);
    }

    // positional arguments
    if(!_args.empty()) {
        std::cout << "\n\nPositional arguments:";
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
            ret += " <" + _args.at(arg).getName() + ">";
            // TODO: Add if statement here to deal with multiple arguments
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

bool params::Params::parsePositionalArgs(int i, int argc, char ** argv) {

    for(int arg_i = 0; i < argc; arg_i++) {
        if(_singleDashBehavior == ERROR && isFlag(argv[i])) {
            usage();
            return false;
        }
        _args[_argOrder[i]].setValue(std::string(argv[i++]));
    }
    return true;
}

bool params::Params::parseArgs(int argc, char** argv)
{
    if(_programName.empty())
        _programName = std::filesystem::path(std::string(argv[0])).filename();
    for(int i = 1; i < argc; i++)
    {
        if(isFlag(std::string(argv[i])))
        {
            std::string option = std::string(argv[i]);
            if(isOption(option))
            {
                std::string value;
                std::string key = _optionKeys.at(Option::parseOption(option));
                if(_options.at(key).getAction() == Option::ACTION::HELP) {
                    printHelp();
                    return false;
                }
                if(_options.at(key).getAction() == Option::ACTION::VERSION) {
                    printVersion();
                    return false;
                }

                if(_options.at(key).getType() == Option::TYPE::BOOL &&
                   _options.at(key).getAction() != Option::ACTION::NONE) {
                    value = "";
                }
                else {
                    if(i + 1 >= argc || !isFlag(std::string(argv[++i]))) {
                        value = std::string(argv[i]);
                    } else {
                        std::cerr << "ERROR: Argument required for " << option << "'\n";
                        return false;
                    }
                }
                if(!_options.at(key).setValue(value)) {
                    std::cerr << "ERROR: '" << value << "' is an invalid argument for '" << option << "'\n";
                    return false;
                }
            } else {
                std::cerr << "ERROR: '" << option << "' is an unknown option\n";
            }
        }
        else if (strcmp(argv[i], "-") == 0) {
            switch (_singleDashBehavior) {
                case START_POSITIONAL:
                    return parsePositionalArgs(i, argc, argv);
                case ERROR:
                    usage();
                    return false;
            }
        }
        else { // we are in positional arguments
            return parsePositionalArgs(i, argc, argv);
        }
    }
    return true;
}

