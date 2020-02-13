cmake_minimum_required(VERSION 3.15)
project(cpp_threadpool)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(cppThreadPool SHARED ${SRC_LIST} ${HEADERS})
#add_executable(cpp_threadpool ${SRC_LIST} ${HEADERS} TestTask.cpp TestTask.h Test.cpp)