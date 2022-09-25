//
// Created by Aaron Maurais on 9/25/22.
//

#include <iostream>

#include <testing.hpp>
#include <tsvFile.hpp>

START_TEST("tsvFile.hpp")
    START_SECTION("Stand alone functions")
        EXPECT_EQUAL(summarize::numDigits(0), 1)
        EXPECT_EQUAL(summarize::numDigits(9), 1)
        EXPECT_EQUAL(summarize::numDigits(10), 2)
        EXPECT_EQUAL(summarize::numDigits(11), 2)
        EXPECT_EQUAL(summarize::numDigits(100), 3)
    END_SECTION
END_TEST
