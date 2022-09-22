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
    extern std::string test_name;
    extern std::string section_name;
    extern int test_section_count;
    extern int test_count;
    extern int test_pass_count;
    extern int test_section_pass_count;

    template <typename T1, typename T2>
    void assertEqual(const char* file, int line,
                     const T1& lhs, const char* lhs_str,
                     const T2& rhs, const char* rhs_str)
    {
        ++test_section_count;
        this_test = bool(lhs == rhs);
        test = test && this_test;
        if(this_test) {
            std::cout << " + line ";
            ++test_section_pass_count;
        } else {
            std::cout << " - line ";
        }
        std::cout << line << ": TEST_EQUAL(" << lhs_str << ", " << rhs_str
                  << ") -> got '" << lhs << "', expected '" << rhs << "'\n";
    }
}

#define ASSERT_EQUAL(lhs, rhs) Test::assertEqual(__FILE__, __LINE__, (lhs), (# lhs), (rhs), (# rhs));

#define START_TEST(nameOfTest) \
    bool Test::all_tests = true;           \
    bool Test::test = true;                \
    bool Test::this_test = true;           \
    int Test::test_count = 0;  \
    int Test::test_section_count = 0;       \
    int Test::test_pass_count = 0;         \
    int Test::test_section_pass_count = 0; \
    std::string Test::test_name = # nameOfTest; \
    std::string Test::section_name;                           \
    int main(int argx, char** argv) \
    {                          \
       std::cout << "Running test of " << Test::test_name << std::endl;                 \
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
    }        \
    std::cout << Test::test_count << " tests of " << Test::test_name << " run. " \
              << Test::test_pass_count  << " passed, " \
              << Test::test_count - Test::test_pass_count << " failed." << std::endl; \
    if(!Test::all_tests) return 1;                                         \
    return 0;    \
    }

#define START_SECTION(nameOfSection) \
Test::test = true;                   \
Test::test_section_count = 0;         \
Test::test_section_pass_count = 0; \
Test::section_name = # nameOfSection; \
std::cout << "Checking " << Test::section_name << " ... " << std::endl; \
try {

#define END_SECTION \
    }                \
    catch ( std::exception& e) \
    { \
      Test::this_test = false; \
      Test::test = false;      \
      Test::all_tests = false; \
      std::cout << "ERROR: Caught unexpected std::exception" << std::endl; \
      std::cout << "\tMessage: " << e.what() << std::endl; \
    }               \
    Test::all_tests = Test::all_tests && Test::test;                    \
    Test::test_pass_count += Test::test_section_pass_count;                \
    Test::test_count += Test::test_section_count;                \
    std::cout << Test::test_section_count << " tests run in section. " << Test::test_section_pass_count  << " passed, " \
              << Test::test_section_count - Test::test_section_pass_count << " failed." << std::endl;

#endif // TESTING_HPP
