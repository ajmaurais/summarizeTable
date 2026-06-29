cmake_minimum_required(VERSION 3.18)
project(summarize)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_PARQUET "Build with Apache Arrow parquet support" ON)
if(ENABLE_PARQUET)
    find_package(Arrow REQUIRED)
    find_package(Parquet REQUIRED)
endif()

option(RUN_TESTS "Run unit tests?" ON)
if(RUN_TESTS)
    message("Running tests...")
    enable_testing()
    add_subdirectory(test)
endif()

set(SUMMARIZE_SOURCES src/main.cpp src/argparse.cpp src/tsvFile.cpp)
if(ENABLE_PARQUET)
    list(APPEND SUMMARIZE_SOURCES src/parquetFile.cpp)
endif()

add_executable(summarize ${SUMMARIZE_SOURCES})

# add_executable(scratch src/test.cpp)

target_include_directories(summarize PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ENABLE_PARQUET)
    target_link_libraries(summarize PRIVATE Parquet::parquet_shared Arrow::arrow_shared)
    target_compile_definitions(summarize PRIVATE ENABLE_PARQUET)
endif()
