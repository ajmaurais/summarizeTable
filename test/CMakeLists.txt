cmake_minimum_required(VERSION 3.18)
project(summarize)

set(CMAKE_CXX_STANDARD 17)

macro(add_test_target TEST_NAME)
    set(TARGET "test_${TEST_NAME}")
    add_executable(${TARGET} ${ARGN})
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    add_test(${TEST_NAME} ${TARGET})
endmacro()

add_test_target(ArgumentParser ${CMAKE_CURRENT_SOURCE_DIR}/../src/argparse.cpp src/test_ArgumentParser.cpp)
add_test_target(TsvFile ${CMAKE_CURRENT_SOURCE_DIR}/../src/tsvFile.cpp src/test_TsvFile.cpp)

