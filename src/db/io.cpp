//
// Created by sven on 17-2-19.
//

#include <stdexcept>
#include <fstream>
#include "io.h"
#include "util.h"

using std::ios;
using DBConst::BLOCK_SIZE;

void IO::write_block(const std::string &data, size_t block_num){
    std::ofstream out(file_path, ios::out | ios::binary);
    if (!out.is_open()){
        throw std::runtime_error("Error: can't found file");
    }
    out.seekp(block_num*BLOCK_SIZE);
    out.write(data.c_str(), BLOCK_SIZE);
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
    std::string data;
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        if (!block_buffer[i]) {
            break;
        }
        data += block_buffer[i];
    }
    in.close();
    return data;
}
