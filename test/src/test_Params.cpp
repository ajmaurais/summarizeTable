//
// Created by Aaron Maurais on 9/21/22.
//

#include <iostream>

#include <testing.hpp>
#include <params.hpp>

START_TEST("Params")

START_SECTION("Test individual options")

params::Option bool_option("b", "bool", "A boolean option", params::Option::VALUE_TYPE::BOOL);
ASSERT_EQUAL(bool_option.isValid(), true)
ASSERT_EQUAL(bool_option.isValid("true"), true)
ASSERT_EQUAL(bool_option.isValid("TRUE"), true)
ASSERT_EQUAL(bool_option.isValid("True"), true)
ASSERT_EQUAL(bool_option.isValid("1"), true)
ASSERT_EQUAL(bool_option.isValid("0"), true)
ASSERT_EQUAL(bool_option.isValid("false"), true)
ASSERT_EQUAL(bool_option.isValid("FALSE"), true)
ASSERT_EQUAL(bool_option.isValid("False"), true)
ASSERT_EQUAL(bool_option.isValid("poop"), false)
ASSERT_EQUAL(bool_option.isValid("2"), false)

ASSERT_EQUAL(bool_option.setValue("true"), true)
ASSERT_EQUAL(bool_option.isValid(), true)
ASSERT_EQUAL(bool_option.setValue("false"), true)
ASSERT_EQUAL(bool_option.isValid(), true)
            ASSERT_EQUAL(bool_option.setValue("pee"), false)
            ASSERT_EQUAL(bool_option.isValid(), false)

END_SECTION

END_TEST