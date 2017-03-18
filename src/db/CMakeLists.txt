cmake_minimum_required(VERSION 3.5)
project(main)

add_compile_options(-std=c++14)

file(GLOB SOURCE_FILES "*.cpp")
add_executable(main ${SOURCE_FILES})
target_link_libraries(main -lboost_system -lboost_filesystem -lcppformat)
