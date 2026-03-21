cmake_minimum_required(VERSION 3.14)
project(Mutex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_mutex main.cpp)
target_link_libraries(test_mutex PRIVATE pthread)
