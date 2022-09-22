
#include <params.hpp>

// const std::regex params::Option::BOOL_PATTERN = std::regex("^(true|false|0|1)$", std::regex_constants::icase);
// const std::regex params::Option::INT_PATTERN = std::regex("^[-+]?[0-9]+$");
// const std::regex params::Option::FLOAT_PATTERN = std::regex("^[-+][0-9]+(\\.[0-9]*)?$");

params::Option::Option(std::string shortOpt, std::string longOpt, std::string help,
               Option::VALUE_TYPE valueType, std::string defaultVal,
               Option::ACTION action) {
    _shortOpt = shortOpt;
    _longOpt = longOpt;
    _help = help;
    _isPositional = false;
    _isSet = false;
    _valueType = valueType;
    // validate default arg if necessary
    if (!defaultVal.empty() && !isValid(defaultVal))
        throw std::runtime_error(defaultVal + " is an invalid value for VALUE_TYPE!");
    _value = defaultVal;
    _action = action;
}

params::Option::Option(std::string name, std::string help, Option::VALUE_TYPE valueType) {
    _isPositional = true;
    _isSet = false;
    _name = name;
    _help = help;
    _valueType = valueType;
    _action = ACTION::NONE;
}
//! Check if \p value can be converted into \p params::Option::valueType.
bool params::Option::isValid(std::string value) const {
    if(_valueType == VALUE_TYPE::BOOL) {
        return std::regex_match(value, std::regex("^(true|false|0|1)$", std::regex_constants::icase));
    }
    if(_action != NONE)
        return false; // Has to be NONE unless _valueType is BOOL.
    if(_valueType == VALUE_TYPE::STRING) return true; // it's already a string so it's valid.
    if(_valueType == VALUE_TYPE::INT) {
        return std::regex_match(value, std::regex("^[-+]?[0-9]+$"));
    }
    if(_valueType == VALUE_TYPE::FLOAT) {
        return std::regex_match(value, std::regex("^[-+][0-9]+(\\.[0-9]*)?$"));
    }
    return false;
}

bool params::Option::isValid() const {
    if(_isPositional && !_isSet) return false;
    if(!_isPositional && _value.empty()) return true;
    return isValid(_value);
}

bool params::Option::setValue(std::string value) {
    if(_valueType == VALUE_TYPE::BOOL && _action != ACTION::NONE){
        if(!value.empty()) return false;
        if(_action == ACTION::STORE_TRUE) _value = "true";
        if(_action == ACTION::STORE_FALSE) _value = "false";
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

