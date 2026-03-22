cmake_minimum_required(VERSION 3.14)
project(MPSC_LockFree_Queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mpsc_queue STATIC mpsc_queue.cpp)

target_include_directories(mpsc_queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(producer producer.cpp)
target_link_libraries(producer PRIVATE mpsc_queue rt pthread)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer PRIVATE mpsc_queue rt pthread)
