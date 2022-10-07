//
// Created by Aaron Maurais on 9/25/22.
//

#include <iostream>
#include <vector>
#include <map>

#include <variant>

// template <typename T>
// class Base {
// private:
//     std::string name;
//     T value;
// public:
//     Base() {};
//     Base(T v) {
//         value = v;
//     }
//
//     void setValue(std::string n, T v) {
//         value = v;
//         name = n;
//     }
//     T getValue() const {
//         return value;
//     }
//     bool isValid() const;
// };
//
// template <typename T> bool Base<T>::isValid() const {
//     throw std::runtime_error("Not implemented for type!");
// }
//
// template <> bool Base<int>::isValid() const {
//     return true;
// }
//
// class Args {
// private:
// public:
//     // std::map<std::string, std::shared_ptr<void> > args;
//     std::map<std::string, void*> args;
//     Args () {}
//     // ~Args
//
//     template <typename T> void addArg(std::string name, T value) {
//         args[name] = new Base<T>(value);
//         // args[name] = std::make_shared<Base<T> >(value);
//     }
//
//     template <typename T> T getArg(const std::string& key) const {
//         // return *(T*)args.at(key);
//         // auto v = std::static_pointer_cast<T>(args.at(key));
//         return ((Base<T>*)(args.at(key)))->getValue();
//     }
// };

class Value{
public:
    typedef std::variant<int, size_t, double, float, std::string, char, bool> ValueType;
private:
    ValueType value;
public:
    Value() {}
    template <typename T> Value(T v) {
        value = v;
    }
    template <typename T> Value() {
        value = T();
    }
    template <typename T> void setValue(T v) {
        value = v;
    }
    template <typename T> void setValue() {
        value = T();
    }
    template <typename T> bool isValid() {
        // const int* i = std::get_if<T>(&value);
        T v = std::get<T>(value);
        return std::holds_alternative<T>(value);
    }
};

class Param {
    Value value;
public:
    Param () {}
    template <typename T> Param(T val) {
        setValue<T>(val);
    }

    template<typename T> void setValue(T v) {
        value.setValue(v);
    }
};

// template<typename T> struct are_same_template : public std::false_type {};

// template<class T1, typename T2> struct are_same_template<std::vector<T1>, T2> : std::true_type {};
// template <template <typename ...> class Container, typename T1, typename T2>
// struct are_same_template<Container<T1>, Container<T2> > : public std::true_type{};
//
// template <template<class...> class T, class T1, class T2>
// struct are_same_template<T<T1>, T<T2> > : std::true_type
// {};

class Argument {
    std::vector<Value> values;
public:
    Argument() {}
};

int main(int argc, char** argv) {
    // Args args;
    // args.addArg<std::string>("arg1", "foo");
    // args.addArg<int>("arg3", 9);

    // std::cout << "arg1 = " << *(std::string *)args.args["arg1"] << std::endl;
    // std::cout << "arg1 = " << args.getArg<std::string>("arg1") << std::endl;
    // std::cout << "arg2 = " << args.getArg<int>("arg3") << std::endl;

    Value v1(1);
    Value v2("poop");
    Param p(char('p'));
    Param p2(int(78));
    Param p3(long(78));
    float f = float();
    Value v3;
    v3.setValue<float>(float());
    v3.setValue<size_t>(1);
    v3.setValue<double>();

    // std::cout << std::boolalpha << v3.isValid<std::string>() << std::endl
    //           << v3.isValid<unsigned long long>()  << std::endl;

    // Value v3(std::pair<int, int>(1, 2));

    return 0;
}
