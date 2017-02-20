#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <utility>

#include "../index/bpTree.h"
#include "table.h"
#include "util.h"
#include "io.h"


using std::ios;
using std::vector;
using DB::Const::BLOCK_SIZE;
using DB::Type::Pos;


std::string Table::get_file_abs_path(bool is_index)const {
    namespace bf = boost::filesystem;
    auto dir_path = bf::path(__FILE__).parent_path();
    std::string file_path = dir_path.generic_string()+"/db_file/";
    if (is_index) {
        file_path += property.table_name + "_index.sdb";
    } else {
        file_path += property.table_name + ".sdb";
    }
    return file_path;
}

RecordList Table::read_record(const std::vector<Pos> &pos_lst)const{
    std::map<size_t, std::vector<Pos>> block_offsets_map;
    for (auto &&item : pos_lst) {
        size_t block_num = item / BLOCK_SIZE;
        Pos offset = item % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(offset);
    }
    IO io(get_file_abs_path(false));
    size_t record_size = property.get_record_size();
    RecordList record_lst(property);
    for (auto &&item: block_offsets_map) {
        std::string data_block = io.read_block(item.first);
        for (auto &&offset : item.second) {
            std::string record_tuple(data_block, offset, record_size);
            record_lst.record_tuple_lst.push_back(record_tuple);
        }
    }
    return record_lst;
}

void Table::write_record(const RecordList &record_lst,
                         const std::vector<Pos> &pos_lst) {
    if (record_lst.record_tuple_lst.size() != pos_lst.size()) {
        throw std::runtime_error("Error: Table::write_record: "
                                         "record_lst and pos_lst length not equal.");
    }
    std::map<size_t, std::vector<std::pair<Pos, std::string>>> block_offsets_map;
    for (size_t i = 0; i < pos_lst.size(); ++i) {
        size_t block_num = pos_lst[i] / BLOCK_SIZE;
        Pos offset =  pos_lst[i] % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(std::make_pair(offset, record_lst.record_tuple_lst[i]));
    }
    IO io(get_file_abs_path(false));
    size_t record_size = property.get_record_size();
    for (auto &&item : block_offsets_map) {
        char *block_buffer = const_cast<char*>(io.read_block(item.first).c_str());
        for (auto &&second : item.second) {
            second.second.copy(block_buffer+second.first, second.second.size());
        }
        io.write_block(block_buffer, item.first);
    }
}
