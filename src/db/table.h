#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>
#include <cppformat/format.h>

#include "record.h"
#include "util.h"

class Table {
public:
    // type
    using Value = DB::Type::Value;
    using Tuple = DB::Type::Tuple;
    using TupleLst = DB::Type::TupleLst;
    using TableProperty = DB::Type::TableProperty;

    //
    Table()= delete;
    Table(const std::string &table_name){
        read_meta_data(table_name);
    }
    // sql
    static void create_table(const DB::Type::TableProperty &property);
    static void drop_table(const std::string &table_name);
    void insert(const Tuple &tuple);
    void update(const std::string &col_name,
                const std::string &op,
                const DB::Type::Value &value);
    void remove(const std::string &col_name,
                const std::string &predicate,
                const DB::Type::Value &value);
    void find(const std::string &col_name,
              const DB::Type::Value &value);
    TupleLst find(const std::string &col_name, std::function<bool(Value)> predicate);

    // === value_lst ===

private:
    DB::Type::TableProperty property;

    void read_meta_data(const std::string &table_name);
    static void write_meta_data(const DB::Type::TableProperty &property);
};

#endif
