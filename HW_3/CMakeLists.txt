cmake_minimum_required(VERSION 3.6)
project(Operation_Systems_234123)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")

set(SOURCE_FILES Factory.cxx hw3_test4.cpp)

add_executable(HW_3 ${SOURCE_FILES})
