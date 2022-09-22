//
// Created by Aaron Maurais on 9/21/22.
//

#ifndef TESTING_HPP
#define TESTING_HPP

#include <iostream>

namespace Test {

    extern bool this_test;
    extern bool test;
    extern bool all_tests;

    template <typename T1, typename T2>
    void assertEqual(const char* file, int line,
                     const T1& lhs, const char* lhs_str,
                     const T2& rhs, const char* rhs_str)
    {
        this_test = bool(lhs == rhs);
        test = test && this_test;
        if(this_test) {
            std::cout << " + line " << line << ": TEST_EQUAL(" << lhs_str << ", " << rhs_str
                      << ") -> got '" << lhs << "', expected '" << rhs << "'\n";
        }
    }
}

#define ASSERT_EQUAL(lhs, rhs) Test::assertEqual(__FILE__, __LINE__, (lhs), (# lhs), (rhs), (# rhs));

#define START_TEST(test_name) \
    bool Test::all_tests = true;           \
    bool Test::test = true;                \
    bool Test::this_test = true; \
    int main(int argx, char** argv) \
    {                               \
       try {

#define END_TEST \
    }             \
    catch ( std::exception& e) \
    { \
      Test::this_test = false; \
      Test::test = false;      \
      Test::all_tests = false; \
      std::cout << "ERROR: Caught unexpected std::exception" << std::endl; \
      std::cout << "\tMessage: " << e.what() << std::endl; \
    }            \
    if(!Test::all_tests) return 1;                                         \
    return 0;    \
    }

#endif // TESTING_HPP
