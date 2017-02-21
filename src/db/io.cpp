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

void IO::write_block(const DB::Type::Bytes &data, size_t block_num){
    std::ofstream out(file_path, ios::out | ios::binary);
    if (!out.is_open()){
        throw std::runtime_error("Error: can't found file");
    }
    out.seekp(block_num*BLOCK_SIZE);
    out.write(data.data(), BLOCK_SIZE);
    out.close();
}

DB::Type::Bytes IO::read_block(size_t block_num) {
    // read bytes block
    std::ifstream in(file_path, ios::in | ios::binary);
    if (!in.is_open()){
        throw std::runtime_error("Error: can't found file");
    }
    in.seekg(block_num*BLOCK_SIZE);
    char block_buffer[BLOCK_SIZE];
    in.read(block_buffer, BLOCK_SIZE);
    DB::Type::Bytes data(std::begin(block_buffer), std::end(block_buffer));
    in.close();
    return data;
}
