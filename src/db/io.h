//
// Created by sven on 17-2-19.
//

#ifndef IO_H
#define IO_H

#include <string>
#include "util.h"

class IO {
public:
    // type
    using Bytes = SDB::Type::Bytes;

    IO(const std::string &file):file_path(get_db_file_path(file)){}

    // file
    static void create_file(const std::string &file_name);
    static void delete_file(const std::string &file_name);
    SDB::Type::Bytes read_file();
    void write_file(const SDB::Type::Bytes &data);

    // block
    SDB::Type::Bytes read_block(size_t block_num);
    void write_block(const SDB::Type::Bytes &data, size_t block_num);

    // get
    size_t get_file_size()const;

    static std::string get_db_file_dir_path();
    static std::string get_db_file_path(const std::string &file_name);
    static void create_dir(const std::string &dir_path);
    static void remove_dir(const std::string &dir_path);

private:
    std::string file_path;
};

#endif
