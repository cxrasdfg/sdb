cmake_minimum_required(VERSION 3.5)
project(main)

set(cmake_cxx_flags ${cmake_cxx_flags} -std=c++14)

set(SOURCE_FILES main.cpp)
add_executable(main ${SOURCE_FILES})
