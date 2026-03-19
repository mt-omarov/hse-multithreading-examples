cmake_minimum_required(VERSION 3.14)
project(hw_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip
)
FetchContent_MakeAvailable(googlebenchmark)

add_library(apply_function INTERFACE)
target_include_directories(apply_function INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE gtest gtest_main apply_function pthread)
enable_testing()
add_test(NAME tests COMMAND tests)

add_executable(benchmarks benchmarks.cpp)
target_link_libraries(benchmarks PRIVATE benchmark benchmark_main apply_function pthread)
