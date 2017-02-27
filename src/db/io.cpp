#include <stdexcept>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "io.h"
#include "util.h"

using std::ios;
using DB::Const::BLOCK_SIZE;
using DB::Type::Bytes;

void IO::write_block(const DB::Type::Bytes &data, size_t block_num){
    if (data.size() != BLOCK_SIZE) {
        throw std::runtime_error("Error: data size not equal BLOCK_SIZE");
    }
    int fd = open(file_path.data(), O_RDWR);
    if (fd < 0) {
        throw std::runtime_error(
            std::string("Error: open file error:")+file_path
        );
    }
    if (get_file_size() < (block_num*BLOCK_SIZE)) {
        lseek(fd, BLOCK_SIZE*(1+block_num), SEEK_SET);
        write(fd, "", 1);
    }
    char *buff = (char*)mmap(nullptr, BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd, BLOCK_SIZE*block_num);
    std::memcpy(buff, data.data(), BLOCK_SIZE);
    close(fd);
    munmap(buff, BLOCK_SIZE);
}

DB::Type::Bytes IO::read_block(size_t block_num) {
    int fd = open(file_path.data(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error(
            std::string("Error: open file error:")+file_path
        );
    }
    char *buff = (char*)mmap(nullptr, BLOCK_SIZE, PROT_READ, MAP_SHARED, fd, BLOCK_SIZE*block_num);
    Bytes bytes(buff, buff+BLOCK_SIZE);
    close(fd);
    munmap(buff, BLOCK_SIZE);
    return bytes;
}

size_t IO::get_file_size() const {
    struct stat file_info;
    stat(file_path.data(), &file_info);
    return (size_t)(file_info.st_size);
}
