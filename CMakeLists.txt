cmake_minimum_required(VERSION 3.15)
project(automannn_threadpool)

add_subdirectory(tast)
add_subdirectory(thread)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/)

#message(WARNING ${PROJECT_SOURCE_DIR})
link_directories(lib)
add_executable(automannn_threadpool main.cpp)
target_link_libraries(automannn_threadpool cppThreadPool.dll cppThreadPoolTask.dll)

