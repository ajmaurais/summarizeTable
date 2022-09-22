
#include <params.hpp>

Option::Option(std::string shortOpt, std::string longOpt, std::string help,
               Option::VALUE_TYPE valueType, std::string defaultVal) {
    _isPositional = false;
    _isSet = false;
    _valueType = valueType;
    // validate default arg if necessary
    if (defaultVal != "" && !isValid(defaultVal))
        throw std::runtime_error(defaultVal + " is an invalid value for VALUE_TYPE!");

}

Option::Option(std::string name, std::string help, Option::VALUE_TYPE valueType) {
    _isPositional = true;
    _isSet = false;
    _name = name;
    _help = help;
    _valueType = valueType;
}
bool Option::isValid(std::string value) const {
    if(_valueType == VALUE_TYPE::BOOL) {

    }
    return true;
}

bool Option::isValid() const {
    if(_isPositional && !_isSet)
        return false;
    return isValid(_value);
}

bool Option::setValue(std::string) {

    return isValid();
}

