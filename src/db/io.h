//
// Created by sven on 17-2-19.
//

#ifndef IO_H
#define IO_H

#include <string>
#include "util.h"

class IO {
public:
    IO()= delete;
    IO(const std::string &file):file_path(file){}

    DB::Type::Bytes read_block(size_t block_num);
    void write_block(const DB::Type::Bytes &data, size_t block_num);
    size_t get_file_size()const;

private:
    std::string file_path;
};


#endif
