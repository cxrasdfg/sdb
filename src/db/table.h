#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>
#include <cppformat/format.h>

#include "record.h"
#include "util.h"

class Table {
public:
    Table()= delete;
    Table(const std::string &table_name){
        read_meta_data(table_name);
    }
    // sql
    static void create_table(const DB::Type::TableProperty &property);
    static void drop_table(const std::string &table_name);
    void insert(const std::vector<std::string> &col_value_lst);
    void update(const std::string &col_name,
                const std::string &op,
                const std::string &value);
    void remove(const std::string &col_name,
                const std::string &op,
                const std::string &value);

    // tuple
    DB::Type::Bytes values_to_bytes(const std::vector<std::string> &values);

private:
    DB::Type::TableProperty property;

    void read_meta_data(const std::string &table_name);
    static void write_meta_data(const DB::Type::TableProperty &property);
};

#endif
