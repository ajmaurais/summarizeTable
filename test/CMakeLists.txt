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

# ENABLE_PARQUET is defined in the parent scope; the Arrow imported targets are
# available here because find_package() runs before add_subdirectory(test).
if(ENABLE_PARQUET)
    add_test_target(Parquet
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/tsvFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/parquetFile.cpp
        src/test_Parquet.cpp)
    target_link_libraries(test_Parquet Parquet::parquet_shared Arrow::arrow_shared)
endif()

