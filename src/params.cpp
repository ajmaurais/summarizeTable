
#include <params.hpp>

size_t params::Option::maxLineLen = params::MAX_LINE_LEN;
size_t params::Option::indendentLen = params::INDENT_LEN;

void params::Option::_checkOptFlags() const {
    if(_shortOpt.empty() && _longOpt.empty())
        throw std::invalid_argument("Long or short option must be specified!");
    if((!_shortOpt.empty() && _shortOpt[0] == '-') || (!_longOpt.empty() && _longOpt[0] == '-'))
        throw std::invalid_argument("Option flags can not begin with '-'!");
}

params::Option::Option(std::string shortOpt, std::string longOpt, std::string help,
                       Option::TYPE valueType, std::string defaultVal,
                       Option::ACTION action) {
    _shortOpt = shortOpt;
    _longOpt = longOpt;
    _checkOptFlags();
    _name = _longOpt.empty() ? _shortOpt : _longOpt;
    _help = help;
    _isPositional = false;
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

params::Option::Option(std::string name, std::string help, Option::TYPE valueType) {
    _isPositional = true;
    _isSet = false;
    _name = name;
    _help = help;
    _valueType = valueType;
    _action = ACTION::NONE;
}

params::Option::Option(const params::Option& rhs) {
    _shortOpt = rhs._shortOpt;
    _longOpt = rhs._longOpt;
    _name = rhs._name;
    _help = rhs._help;
    _value = rhs._value;
    _defaultValue = rhs._defaultValue;
    _valueType = rhs._valueType;
    _action = rhs._action;
    _isPositional = rhs._isPositional;
    _isSet = rhs._isSet;
}

params::Option::Option() {
    _valueType = TYPE::STRING;
    _action = ACTION::NONE;
    _isPositional = true;
    _isSet = false;
}

params::Option &params::Option::operator=(const params::Option &rhs) {
    _shortOpt = rhs._shortOpt;
    _longOpt = rhs._longOpt;
    _name = rhs._name;
    _help = rhs._help;
    _value = rhs._value;
    _defaultValue = rhs._defaultValue;
    _valueType = rhs._valueType;
    _action = rhs._action;
    _isPositional = rhs._isPositional;
    _isSet = rhs._isSet;
    return *this;
}

//! Check if \p value can be converted into \p params::Option::valueType.
bool params::Option::isValid(std::string value) const {
    if(_valueType == TYPE::BOOL) {
        return std::regex_match(value, std::regex("^(true|false|0|1)$", std::regex_constants::icase));
    }
    if(_action != ACTION::NONE)
        return false; // Has to be NONE unless _valueType is BOOL.
    if(_valueType == TYPE::STRING) return true; // it's already a string so it's valid.
    if(_valueType == TYPE::INT) {
        return std::regex_match(value, std::regex("^-?[0-9]+$"));
    }
    if(_valueType == TYPE::FLOAT) {
        return std::regex_match(value, std::regex("^-?[0-9]+(\\.[0-9]*)?$"));
    }
    return false;
}

bool params::Option::isValid() const {
    if(_isPositional && !_isSet) return false;
    if(_valueType != TYPE::BOOL && _action != ACTION::NONE) return false;
    if(!_isPositional && _value.empty()) return true;
    return isValid(_value);
}

bool params::Option::setValue(std::string value) {
    if(_valueType == TYPE::BOOL && _action != ACTION::NONE){
        if(!value.empty()) return false;
        if(_action == ACTION::STORE_TRUE) _value = "true";
        if(_action == ACTION::STORE_FALSE) _value = "false";
        if(_action == ACTION::HELP) _value = "true";
        if(_action == ACTION::VERSION) _value = "true";
    }
    else if(_isPositional && value.empty()) {
        return false;
    }
    else {
        _value = value;
    }
    bool valid = isValid();
    if(valid) _isSet = true;
    return valid;
}

std::string params::Option::parseOption(std::string arg) {
    std::string ret = arg;
    size_t len = std::min(size_t(2), arg.size());
    for(size_t i = 0; i < len; i++) {
        if(arg[i] == '-') ret.erase(0, 1);
    }
    return ret;
}

std::string params::Option::help() const {
    std::string ret = _help;
    // if(_valueType == TYPE::BOOL && _action != ACTION::NONE)
    if(!_defaultValue.empty() && _action == ACTION::NONE) {
        ret += " '" + _defaultValue + "' is the default.";
    }
    return ret;
}

std::string params::Option::signature(int margin) const
{
    std::string ret = "";
    std::string name = (_action == ACTION::NONE ? _name : "");
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    if(!_shortOpt.empty()) {
        ret += "-" + _shortOpt + (name.empty() ? "" : ' ' + name);
        if(!_longOpt.empty()) ret += ", ";
    }
    if(!_longOpt.empty())
        ret += "--" + _longOpt + (name.empty() ? "" : ' ' + name);

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

std::string params::Option::multiLineString(std::string addStr, size_t margin, bool indentFirstLine) {
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
    if(_options.find(option.getName()) != _options.end())
        throw std::runtime_error("'" + option.getName() + "' already exists as an option!");
    _options[option.getName()] = option;
}

//! Set program version and add version option.
void params::Params::setVersion(std::string version, std::string shortOpt, std::string longOpt) {
    _version = version;
    addOption(shortOpt, longOpt, "Print version and exit", Option::TYPE::BOOL, "false", Option::ACTION::VERSION);
}

void params::Params::addArgument(std::string name, std::string help, params::Option::TYPE valueType) {
    _args.emplace_back(name, help, valueType);
}

bool params::Params::isOption(std::string arg) const {
    if(!isFlag(arg)) return false;
    std::string key = Option::parseOption(arg);
    return _options.find(key) != _options.end();
}

void params::Params::printHelp() const {
    size_t margin = 2;
    std::cout << "Usage: " << signature() << "\n\n" << params::Option::multiLineString(_description, margin);

    //options


    // positional arguments
    if(!_args.empty()) std::cout << "\n\nPositional arguments:";
    for(const auto& arg: _args)
        arg.signature();
}

std::string params::Params::signature() const {
    std::string ret = _programName;
    if(!_options.empty()) ret += " [options]";
    if(!_args.empty()) {
        for(const auto& arg: _args) {
            ret += " <" + arg.getName() + ">";
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

bool params::Params::parseArgs(int argc, char **argv)
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
                if(_options[option].getType() == Option::TYPE::BOOL &&
                   _options[option].getAction() != Option::ACTION::NONE) {
                    value = "";
                }
                else {
                    if(!isFlag(std::string(argv[++i]))) {
                        value = std::string(argv[i]);
                    } else {
                        std::cerr << "ERROR: Argument required for " << option << "'\n";
                        return false;
                    }
                }
                if(!_options[option].setValue(value)) {
                    if(_options[option].getAction() == Option::ACTION::HELP) printHelp();
                    if(_options[option].getAction() == Option::ACTION::VERSION) printVersion();
                    else std::cerr << "ERROR: '" << value << "' is an invalid argument for '" << option << "'\n";
                    return false;
                }
            } else {
                std::cerr << "ERROR: '" << option << "' is an unknown option\n";
            }
        }
        else { // we are in positional arguments
            for(int arg_i = 0; i < argc; arg_i++) {
                if(isFlag(argv[i])) {
                    usage();
                    return false;
                }
                _args[arg_i].setValue(std::string(argv[i++]));
            }
        }
    }
    return true;
}

