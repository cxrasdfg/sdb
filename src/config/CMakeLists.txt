cmake_minimum_required(VERSION 3.5)
project(main)
set(CMAKE_COMMAND ${CMAKE_CURRENT_LIST_DIR})


set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++14)

set(SOURCE_FILES test.cpp config.hpp)
add_executable(main ${SOURCE_FILES})
target_link_libraries(main -lconfig++ -lboost_system -lboost_filesystem)
