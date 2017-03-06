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
#include <boost/filesystem.hpp>

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
    if (get_file_size() < (block_num+1)*BLOCK_SIZE) {
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
    if (get_file_size() < (block_num+1)*BLOCK_SIZE) {
        lseek(fd, BLOCK_SIZE*(1+block_num), SEEK_SET);
        write(fd, "", 1);
    }
    char *buff = (char*)mmap(nullptr, BLOCK_SIZE, PROT_READ, MAP_SHARED, fd, BLOCK_SIZE*block_num);
    Bytes bytes(buff, buff+BLOCK_SIZE);
    close(fd);
    munmap(buff, BLOCK_SIZE);
    return bytes;
}

DB::Type::Bytes IO::read_file() {
    std::ifstream in(file_path, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error(
                std::string("Error: read file [")+file_path+"] error!"
        );
    }
    size_t file_size = get_file_size();
    Bytes buffer_data(file_size);
    in.read(buffer_data.data(), file_size);
    return buffer_data;
}

void IO::write_file(const DB::Type::Bytes &data) {
    std::ofstream out(file_path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error(
                std::string("Error: write file [")+file_path+"] error!"
        );
    }
    out.write(data.data(), data.size());
}

size_t IO::get_file_size() const {
    struct stat file_info;
    stat(file_path.data(), &file_info);
    return (size_t)(file_info.st_size);
}

std::string IO::get_db_file_dir_path() {
    namespace bf = boost::filesystem;
    auto dir_path = bf::path(__FILE__).parent_path();
    std::string file_path = dir_path.generic_string()+"/db_file";
    return file_path;
}

std::string IO::get_db_file_path(const std::string &file_name) {
    using namespace DB::Enum;
    std::string file_path = get_db_file_dir_path();
    return file_path + '/' + file_name;
}
