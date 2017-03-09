cmake_minimum_required(VERSION 3.5)
project(main)

add_compile_options(-std=c++14)

set(SOURCE_FILES main.cpp table.h record.h table.cpp io.cpp io.h util.h bpTree.h util.cpp record.cpp bpTree.cpp)
add_executable(main ${SOURCE_FILES})
target_link_libraries(main -lboost_system -lboost_filesystem -lcppformat)
