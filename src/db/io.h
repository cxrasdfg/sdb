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
    IO(const std::string &file):file_path(get_db_file_path(file)){}

    // block
    DB::Type::Bytes read_block(size_t block_num);
    void write_block(const DB::Type::Bytes &data, size_t block_num);

    // file
    DB::Type::Bytes read_file();
    void write_file(const DB::Type::Bytes &data);

    // get
    size_t get_file_size()const;
    std::string get_db_file_dir_path();
    std::string get_db_file_path(const std::string &file_name);

private:
    std::string file_path;
};

#endif
