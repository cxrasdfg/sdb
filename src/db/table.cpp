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
using DB::Type::Pos;
using DB::Type::PosList;
using DB::Type::Bytes;
using DB::Type::Value;
using DB::Type::TupleProperty;
using DB::Enum::ColType;
using namespace DB::Const;

// SQL
void Table::insert(const Tuple &tuple) {
    auto bytes = Record::tuple_to_bytes(tuple);
    BpTree bpTree(property);
    bpTree.insert(tuple.get_col_value(property.tuple_property, property.key), bytes);
}

void Table::update(const std::string &pred_col_name, DB::Type::BVFunc predicate,
                   const std::string &op_col_name, DB::Type::VVFunc op) {
    Record record(property);
    BpTree bpTree(property);
    bool is_var_type = DB::Function::is_var_type(property.tuple_property.get_col_type(op_col_name));
    if (!is_has_index(pred_col_name) && !is_var_type) {
        record.update(pred_col_name, predicate, op_col_name, op);
        return;
    }
    // get tuple lst
    TupleLst tuple_lst(property.tuple_property);
    if (is_has_index(pred_col_name)) {
        PosList pos_lst = bpTree.find(predicate);
        tuple_lst = record.read_record(pos_lst);
    } else {
        tuple_lst = record.find(pred_col_name, predicate);
    }
    // data update
    for (auto &&tuple : tuple_lst.tuple_lst) {
        Value key = tuple.get_col_value(property.tuple_property, property.key);
//        Value value = op(tuple.get_col_value(property.tuple_property, pred_col_name));
        tuple.set_col_value(tuple_lst.tuple_property, pred_col_name, predicate, op_col_name, op);
        Bytes data = Record::tuple_to_bytes(tuple);
        bpTree.update(key, data);
    }
}

void Table::remove(const std::string &col_name, const Value &value) {
    BpTree bpTree(property);
    if (is_has_index(col_name)) {
        bpTree.remove(value);
        return;
    }
    TupleLst tuple_lst = find(col_name, value);
    for (auto &&tuple : tuple_lst.tuple_lst) {
        Value key = tuple.get_col_value(property.tuple_property, property.key);
        bpTree.remove(key);
    }
}

void Table::remove(const std::string &col_name, std::function<bool(Value)> predicate) {
    TupleLst tuple_lst = find(col_name, predicate);
    BpTree bpTree(property);
    for (auto &&tuple : tuple_lst.tuple_lst) {
        Value value = tuple.get_col_value(property.tuple_property, property.key);
        bpTree.remove(value);
    }
}

Table::TupleLst Table::find(const std::string &col_name, const DB::Type::Value &value) {
    Record record(property);
    if (col_name == property.key) {
        BpTree bpTree(property);
        PosList pos_lst = bpTree.find(value);
        return record.read_record(pos_lst);
    }
    auto predicate = DB::Function::get_bvfunc(DB::Enum::EQ, value);
    return record.find(col_name, predicate);
}

Record::TupleLst Table::find(const std::string &col_name, std::function<bool(Value)> predicate) {
    Record record(property);
    if (col_name == property.key) {
        PosList pos_lst;
        BpTree bpTree(property);
        pos_lst = bpTree.find(predicate);
//        for (auto &&pos : pos_lst) {
//            std::cout << pos << " ";
//        }
//        std::cout << std::endl;
        return record.read_record(pos_lst);
    }
    return record.find(col_name, predicate);
}

void Table::create_table(const DB::Type::TableProperty &property) {
    // table meta
    // create dir
    IO::create_dir(property.db_name);
    IO::create_dir(property.db_name+"/"+property.table_name);
    write_meta_data(property);

    //index
    Record::create(property);

    //record
    BpTree::create(property);
}

void Table::drop_table(const TableProperty &property) {
    IO::delete_file(get_table_meta_path(property));
    // index
    BpTree::drop(property);
    //record
    Record::drop(property);
    // drop dir
    IO::remove_dir(property.db_name+"/"+property.table_name);
    IO::remove_dir(property.db_name);
}

// ========= private ========
void Table::read_meta_data(const std::string &db_name, const std::string &table_name) {
    // --- read table name key map ---

    property.db_name = db_name;
    property.table_name = table_name;
    IO io(get_table_meta_path(property));
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
    for (size_t i = 0; i < col_count; ++i) {
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
        property.tuple_property.push_back(col_name, type, type_size);
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
    Bytes tuple_count_bytes = DB::Function::en_bytes(property.tuple_property.property_lst.size());
    bytes.insert(bytes.end(), tuple_count_bytes.begin(), tuple_count_bytes.end());
    for (auto &&item : property.tuple_property.property_lst) {
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
    IO io(get_table_meta_path(property));
    io.write_file(bytes);
}

bool Table::is_has_index(const std::string &col_name) {
    return col_name == property.key;
}

// ========== private function ========
std::string Table::get_table_meta_path(const TableProperty &property) {
    return property.db_name + "/" + property.table_name + "/meta.sdb";
}
