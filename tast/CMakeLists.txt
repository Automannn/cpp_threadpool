cmake_minimum_required(VERSION 3.15)
project(cpp_threadpool_task)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(cppThreadPoolTask SHARED ${SRC_LIST} ${HEADERS})