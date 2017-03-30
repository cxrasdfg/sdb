//
// Created by sven on 17-3-23.
//

#include "db.h"
#include "io.h"
#include "util.h"
#include "table.h"

using namespace SDB;

// ========== Public =======
void DB::create_db(const std::string &db_name) {
    IO::create_dir(db_name);
    IO::create_file(get_meta_path(db_name));
    write_meta_data(db_name, TableNameSet());
}

void DB::drop_db(){
    IO::delete_file(get_meta_path(db_name));
    for (auto &&table_name : table_name_set) {
        Table table(db_name, table_name);
        table.drop_table();
    }
    IO::remove_dir(db_name);
}

void DB::create_table(const SDB::Type::TableProperty &table_property) {
    Table::create_table(table_property);
    table_name_set.insert(table_property.table_name);
}

void DB::drop_table(const std::string &table_name) {
    Table table(db_name, table_name);
    if (table.is_referenced()) {
        throw std::runtime_error("Error:[drop table] table already is referenced");
    }
    for (auto &&item : table.get_referencing_map()) {
        Table ref_table(db_name, item.first);
        ref_table.remove_referenced(table_name);
    }
    table_name_set.erase(table_name);
    table.drop_table();
    is_db_drop = true;
}

void DB::insert(const std::string &table_name, const Tuple &tuple) {
    Table table(db_name, table_name);
    check_referencing(table, tuple);
    table.insert(tuple);
}

void DB::remove(const std::string &table_name,
                const std::string &col_name,
                const Value &value) {
    Table table(db_name, table_name);
    check_referenced(table, value);
    table.remove(col_name, value);
}

void DB::remove(const std::string &table_name,
                const std::string &col_name,
                SDB::Type::BVFunc predicate) {
    Table table(db_name, table_name);
    check_referenced(table, predicate);
    table.remove(col_name, predicate);
}

void DB::update(const std::string &table_name,
                const std::string &pred_col_name, SDB::Type::BVFunc predicate,
                const std::string &op_col_name, SDB::Type::VVFunc op) {
    Table table(db_name, table_name);
    TupleLst tuple_lst = table.find(pred_col_name, predicate);
    bool is_op_key = op_col_name == table.get_key();
    for (auto &&tuple : tuple_lst.tuple_lst) {
        if (is_op_key) {
            Value check_value = tuple.get_col_value(table.get_tuple_property(), op_col_name);
            check_referenced(table, check_value);
        }
        check_referencing(table, tuple);
    }
    table.update(pred_col_name, predicate, op_col_name, op);
}

DB::TupleLst DB::find(const std::string &table_name,
                      const std::string &col_name,
                      const SDB::Type::Value &value) {
    Table table(db_name, table_name);
    return table.find(col_name, value);
}

DB::TupleLst DB::find(const std::string &table_name,
                      const std::string &col_name,
                      std::function<bool(DB::Value)> predicate) {
    Table table(db_name, table_name);
    return table.find(col_name, predicate);
}

// ========== Private =======
void DB::write_meta_data(const std::string &db_name, const TableNameSet &set) {
    Type::Bytes bytes;
    Function::bytes_append(bytes, set);
    IO io(get_meta_path(db_name));
    io.write_file(bytes);
}

void DB::read_meta_data() {
    IO io(get_meta_path(db_name));
    Type::Bytes bytes = io.read_file();
    size_t offset = 0;
    Function::de_bytes(table_name_set, bytes, offset);
}

std::string DB::get_meta_path(const std::string &db_name) {
    return db_name + "/meta.sdb";
}

// check integrity
template <typename T>
void DB::check_referenced(const Table &table, T t){
    auto referenced_map = table.get_referenced_map();
    for (auto &&item : referenced_map) {
        Table ref_table(db_name, item.first);
        Type::TupleLst tuple_lst = ref_table.find(item.second, t);
        if (!tuple_lst.tuple_lst.empty()) {
            throw std::runtime_error(
                std::string("Error : referenced key")
            );
        }
    }
}

void DB::check_referencing(const Table &table, const Type::Tuple &tuple){
    auto referencing_map = table.get_referencing_map();
    for (auto &&item : referencing_map) {
        Table ref_table(db_name, item.first);
        Type::Value check_value = tuple.get_col_value(table.get_tuple_property(), item.second);
        Type::TupleLst tuple_lst = ref_table.find(ref_table.get_key(), check_value);
        if (tuple_lst.tuple_lst.empty()) {
            throw std::runtime_error(
                std::string("Error [db.insert]: can't fonud referencing key:")+check_value.get_string()
            );
        }
    }
}
