#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <boost/filesystem.hpp>

#include "../index/bpTree.h"
#include "table.h"

using std::ios;

std::string Table::get_file_abs_path(bool is_index) {
    namespace bf = boost::filesystem;
    auto dir_path = bf::path(__FILE__).parent_path();
    std::string file_path = dir_path.generic_string()+"/db_file/";
    if (is_index) {
        file_path += get_table_name() + "_index.sdb";
    } else {
        file_path += get_table_name() + ".sdb";
    }
    return file_path;
}

void Table::write_block(const std::vector<std::string> &data_block, int block_num, bool is_index) {
    // open file
    std::string file_path = get_file_abs_path(is_index);
    std::ofstream out(file_path, ios::out | ios::binary);
    if (!out.is_open()){
        throw std::runtime_error("Error: can't found file");
    }
    out.seekp(block_num*get_block_size());

    // convert vector<string> to bytes block
    std::string block_buffer;
    size_t data_block_size = 0;
    for (const auto &str: data_block) {
        data_block_size += str.size();
        if (data_block_size > get_block_size()) {
            throw std::runtime_error("Error: data_block too big");
        }
        block_buffer += str;
        block_buffer += '\0';
    }
    block_buffer += '\0';
    out.write(block_buffer.c_str(), get_block_size());
}

std::vector<std::string> Table::read_block(int block_num, bool is_index) {
    // read bytes block
    std::string file_path = get_file_abs_path(is_index);
    std::ifstream in(file_path, ios::in | ios::binary);
    if (!in.is_open()){
        throw std::runtime_error("Error: can't found file");
    }
    in.seekg(block_num*get_block_size());
    char block_buffer[get_block_size()];
    in.read(block_buffer, get_block_size());
    in.close();

    // convert bytes block to vector<string>
    std::vector<std::string> split_block;
    std::string str;
    int seq_count = 0;
    for (size_t i = 0; i < get_block_size(); ++i) {
        if (block_buffer[i] == '\0') {
            seq_count++;
            if (seq_count == 2) {
                break;
            }
            split_block.push_back(str);
            str.clear();
        } else {
            str += block_buffer[i];
            seq_count = 0;
        }
    }
    return split_block;
}

Record Table::read_record(int block_num){
    std::vector<std::string> split_block = read_block(block_num, false);
}

// getter setter
const std::string &Table::get_table_name() const {
    return table_name;
}

void Table::set_table_name(const std::string &table_name) {
    Table::table_name = table_name;
}

size_t Table::get_block_size() const {
    return block_size;
}

void Table::set_block_size(size_t block_size) {
    Table::block_size = block_size;
}
