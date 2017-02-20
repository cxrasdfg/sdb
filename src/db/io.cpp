//
// Created by sven on 17-2-19.
//

#include <stdexcept>
#include <fstream>
#include "io.h"
#include "util.h"
#include <string>
#include <iostream>

using std::ios;
using DB::Const::BLOCK_SIZE;

void IO::write_block(const char *data, size_t block_num){
    std::cout << "begin" << std::endl;
    std::ofstream out(file_path, ios::out | ios::binary);
    if (!out.is_open()){
        std::cout << "file: " << file_path << std::endl;
        throw std::runtime_error("Error: can't found file");
    }
    std::cout << "begin begin" << std::endl;
    out.seekp(block_num*BLOCK_SIZE);
    out.write(data, BLOCK_SIZE);
    out.close();
}

std::string IO::read_block(size_t block_num) {
    // read bytes block
    std::ifstream in(file_path, ios::in | ios::binary);
    if (!in.is_open()){
        throw std::runtime_error("Error: can't found file");
    }
    in.seekg(block_num*BLOCK_SIZE);
    char block_buffer[BLOCK_SIZE];
    in.read(block_buffer, BLOCK_SIZE);
    std::string data(block_buffer, BLOCK_SIZE);
    in.close();
    return data;
}
