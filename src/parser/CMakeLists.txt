cmake_minimum_required(VERSION 3.5)
project(main)

set(cmake_cxx_flags ${cmake_cxx_flags} -std=c++14)

set(SOURCE_FILES test.cpp lexer.cpp ../config/config.cpp parser.cpp ast.cpp)
add_executable(main ${SOURCE_FILES})
target_link_libraries(main -lboost_system -lboost_filesystem)
