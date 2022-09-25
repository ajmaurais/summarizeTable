cmake_minimum_required(VERSION 3.18)
project(summarize)

set(CMAKE_CXX_STANDARD 17)

option(RUN_TESTS "Run unit tests?" ON)
if(RUN_TESTS)
    message("Running tests...")
    enable_testing()
    add_subdirectory(test)
endif()

add_executable(summarize src/main.cpp src/params.cpp src/tsvFile.cpp)

target_include_directories(summarize PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
