//
// Created by sven on 17-2-19.
//

#ifndef IO_H
#define IO_H

#include <string>

class IO {
public:
    IO()= delete;
    IO(const std::string &file):file_path(file){}

    std::string read_block(size_t block_num);
    void write_block(const std::string &data, size_t block_num);

private:
    std::string file_path;
};


#endif
