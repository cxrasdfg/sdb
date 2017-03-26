#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>

#include "record.h"
#include "util.h"

class Table {
public:
    // type
    using Pos = SDB::Type::Pos ;
    using PosList= SDB::Type::PosList;
    using Value = SDB::Type::Value;
    using Tuple = SDB::Type::Tuple;
    using TupleLst = SDB::Type::TupleLst;
    using TableProperty = SDB::Type::TableProperty;

    //
    Table()= delete;
    Table(const std::string &db_name, const std::string &table_name){
        read_meta_data(db_name, table_name);
    }
    // sql
    static void create_table(const SDB::Type::TableProperty &property);
    static void drop_table(const TableProperty &property);
    void insert(const Tuple &tuple);
    void remove(const std::string &col_name, const Value &value);
    void remove(const std::string &col_name, SDB::Type::BVFunc predicate);
    void update(const std::string &pred_col_name, SDB::Type::BVFunc predicate, const std::string &op_col_name,
                    SDB::Type::VVFunc op);
    TupleLst find(const std::string &col_name, const SDB::Type::Value &value);
    TupleLst find(const std::string &col_name, std::function<bool(Value)> predicate);

private:
    //function
    static std::string get_table_meta_path(const TableProperty &property);

    void read_meta_data(const std::string &db_name, const std::string &table_name);
    static void write_meta_data(const SDB::Type::TableProperty &property);
    bool is_has_index(const std::string &col_name);

private:
    SDB::Type::TableProperty property;
};

#endif
