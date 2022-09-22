cmake_minimum_required(VERSION 3.18)
project(summarize)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_Params src/test_Params.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../src/params.cpp)
target_include_directories(test_Params PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(test_Params PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_test(Params test_Params)
