#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <utility>
#include <functional>

#include "bpTree.h"
#include "table.h"
#include "util.h"
#include "io.h"


using std::ios;
using std::vector;
using DB::Const::BLOCK_SIZE;
using DB::Type::Pos;
using DB::Type::Bytes;
using DB::Type::Value;
using DB::Type::ColProperty;
using DB::Enum::ColType;


// SQL
void Table::insert(const vector<Value> &col_value_lst) {
    auto bytes = values_to_bytes(col_value_lst);
    BpTree bpTree(property);
    bpTree.insert(get_col_value(property.key, col_value_lst), bytes);
}

void Table::update(const std::string &col_name,
                   const std::string &op,
                   const Value &value) {
}

void Table::remove(const std::string &col_name,
                   const std::string &predicate,
                   const Value &value) {
}

void Table::find(const std::string &col_name,
                 const std::string &predicate,
                 const DB::Type::Value &value) {
}

std::function<bool(Value, Value)> Table::make_predicate_func(const std::string &predicate) {
    std::map<std::string, std::function<bool(Value, Value)>> predicate_map{
            {"<", [](Value v1, Value v2){ return v1 < v2;}},
            {"=", [](Value v1, Value v2){ return v1 == v2;}},
            {">", [](Value v1, Value v2){ return v2 < v1;}},
            {"<=", [](Value v1, Value v2){ return v1 <= v2;}},
            {">=", [](Value v1, Value v2){ return !(v1 < v2);}}
    };
    return predicate_map.at(predicate);
}

std::function<Value(Value, Value)> Table::make_op_func(const std::string &op) {
    std::map<std::string, std::function<Value(Value, Value)>> op_map{
            {"+", [](Value v1, Value v2){ return v1 + v2;}},
            {"-", [](Value v1, Value v2){ return v1 - v2;}},
            {"*", [](Value v1, Value v2){ return v2 * v1;}},
            {"/", [](Value v1, Value v2){ return v2 / v1;}}
    };
    return op_map.at(op);
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

    property.table_name = table_name;
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
    offset += size_len;
    for (int i = 0; i < col_count; ++i) {
        // cal name
        size_t col_name_size;
        std::memcpy(&col_name_size, beg+offset, size_len);
        offset += size_len;
        std::string col_name(beg+offset, col_name_size);
        offset += col_name_size;

        // type
        ColType type;
        std::memcpy(&type, beg+offset, sizeof(char));
        offset += sizeof(char);
        // type_size
        size_t type_size;
        std::memcpy(&type_size, beg+offset, sizeof(size_t));
        offset += sizeof(size_t);
        property.col_property.push_back(col_name, type, type_size);
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
    Bytes tuple_count_bytes = DB::Function::en_bytes(property.col_property.tuple_lst.size());
    bytes.insert(bytes.end(), tuple_count_bytes.begin(), tuple_count_bytes.end());
    for (auto &&item : property.col_property.tuple_lst) {
        // col name
        Bytes col_size_bytes = DB::Function::en_bytes(item.col_name.size());
        bytes.insert(bytes.end(), col_size_bytes.begin(), col_size_bytes.end());
        bytes.insert(bytes.end(), item.col_name.begin(), item.col_name.end());
        // type
        bytes.push_back(item.col_type);
        // type size
        Bytes type_size_bytes = DB::Function::en_bytes(item.type_size);
        bytes.insert(bytes.end(), type_size_bytes.begin(), type_size_bytes.end());
    }
    IO io(property.table_name+"_meta.sdb");
    io.write_file(bytes);
}

// ========= tuple_lst =========
DB::Type::Bytes Table::values_to_bytes(const std::vector<DB::Type::Value> &values) {
    Bytes bytes;
    for (auto &&v : values) {
        bytes.insert(bytes.end(), v.data.begin(), v.data.end());
    }
    return bytes;
}

DB::Type::Value Table::get_col_value(const std::string &col_name, const std::vector<Value> &values) {
    auto tuple_lst = property.col_property.tuple_lst;
    for (size_t j = 0; j < tuple_lst.size(); ++j) {
        if (tuple_lst[j].col_name == col_name) {
            return values[j];
        }
    }
    throw std::runtime_error(std::string("Error: can't found col:{")+col_name+"}");
}
