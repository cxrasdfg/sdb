#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <utility>

#include "bpTree.h"
#include "table.h"
#include "util.h"
#include "io.h"


using std::ios;
using std::vector;
using DB::Const::BLOCK_SIZE;
using DB::Type::Pos;
using DB::Type::Bytes;


// SQL
void Table::insert(const vector<std::string> &col_value_lst) {
    Bytes bytes = values_to_bytes(col_value_lst);
}

void Table::update(const std::string &col_name,
                   const std::string &op,
                   const std::string &value) {
}

void Table::remove(const std::string &col_name,
                   const std::string &op,
                   const std::string &value) {

}

void Table::create_table(const DB::Type::TableProperty &property) {
    // table meta
    write_meta_data(property);

    // meta_index.sdb
    IO io(property.table_name+"_meta_index.sdb");
    size_t Pos_len = sizeof(size_t);
    size_t size_len = sizeof(size_t);
    Bytes bytes(Pos_len+size_len+size_len);
    size_t root_pos = 0;
    size_t free_pos_count = 0;
    size_t free_end_pos = 0;
    std::memcpy(bytes.data(), &root_pos, Pos_len);
    std::memcpy(bytes.data()+size_len, &free_pos_count, size_len);
    std::memcpy(bytes.data()+size_len+size_len, &free_end_pos, size_len);
    io.write_file(bytes);

    // meta_record
    IO record_io(property.table_name+"_meta_record.sdb");
    Bytes record_bytes(size_len*2);
    size_t record_free_pos_count = 0;
    size_t record_free_end_pos = 0;
    std::memcpy(&record_free_pos_count, record_bytes.data(), size_len);
    std::memcpy(&record_free_end_pos, record_bytes.data()+size_len, size_len);
    record_io.write_file(record_bytes);
    // index and record file
    IO::create_file(property.table_name+"_record.sdb");
    IO::create_file(property.table_name+"_index.sdb");
}

void Table::drop_table(const std::string &table_name) {
    IO::delete_file(table_name+"_meta.sdb");
    IO::delete_file(table_name+"_meta_index.sdb");
    IO::delete_file(table_name+"_index.sdb");
    IO::delete_file(table_name+"_meta_record.sdb");
    IO::delete_file(table_name+"_record.sdb");
}

// ========= private ========
void Table::read_meta_data(const std::string &table_name) {
    // --- read table name key map ---

    IO io(table_name+"_meta.sdb");
    Bytes bytes = io.read_file();
    auto beg = bytes.data();
    size_t offset = 0;
    // table name size
    size_t size_len = sizeof(size_t);
    // key name size
    size_t key_name_size;
    std::memcpy(&key_name_size, beg+offset, size_len);
    offset += size_len;
    property.key = std::string(beg+offset, key_name_size);
    offset += key_name_size;

    size_t col_count;
    std::memcpy(&col_count, beg+offset, size_len);
    std::map<std::string, std::pair<char, size_t>> map;
    offset += size_len;
    for (int i = 0; i < col_count; ++i) {
        // cal name
        size_t col_name_size;
        std::memcpy(&col_name_size, beg+offset, size_len);
        offset += size_len;
        std::string col_name(beg+offset, col_name_size);
        offset += col_name_size;

        // type
        char type;
        std::memcpy(&type, beg+offset, sizeof(char));
        offset += sizeof(char);
        // type_size
        size_t type_size;
        std::memcpy(&type_size, beg+offset, sizeof(size_t));
        offset += sizeof(size_t);
        map[col_name] = std::make_pair(type, type_size);
    }
}

void Table::write_meta_data(const DB::Type::TableProperty &property) {
    // write table_name key map

    Bytes bytes;
    // key
    Bytes key_size_bytes = DB::Function::en_bytes(property.key.size());
    bytes.insert(bytes.end(), key_size_bytes.begin(), key_size_bytes.end());
    bytes.insert(bytes.end(), property.key.begin(), property.key.end());
    //map
    Bytes map_count_bytes = DB::Function::en_bytes(property.col_property.size());
    bytes.insert(bytes.end(), map_count_bytes.begin(), map_count_bytes.end());
    for (auto &&item : property.col_property) {
        // col name
        Bytes col_size_bytes = DB::Function::en_bytes(item.first.size());
        bytes.insert(bytes.end(), col_size_bytes.begin(), col_size_bytes.end());
        bytes.insert(bytes.end(), item.first.begin(), item.first.end());
        // type
        bytes.push_back(item.second.first);
        // type size
        Bytes type_size_bytes = DB::Function::en_bytes(item.second.second);
        bytes.insert(bytes.end(), type_size_bytes.begin(), type_size_bytes.end());
    }
    IO io(property.table_name+"_meta.sdb");
    io.write_file(bytes);
}

// ========= tuple =========
DB::Type::Bytes Table::values_to_bytes(const std::vector<std::string> &values) {
    if (values.size() != property.col_property.size()) {
        throw std::runtime_error("Error: col values count error");
    }
    auto col_name_lst = property.col_name_lst;
    Bytes bytes;
    for (int i = 0; i < col_name_lst.size(); ++i) {
        auto item = property.col_property.at(col_name_lst[i]);
        Bytes data(item.second);
        std::memcpy(data.data(), values[i].data(), item.second);
        bytes.insert(bytes.end(), data.begin(), data.end());
    }
    return bytes;
}
