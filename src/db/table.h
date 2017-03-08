#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>
#include <initializer_list>
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
//    template <typename T, typename ...Args>
//    static DB::Type::Bytes make_tuple(const T &t, const Args& ...args);
    static void create_table(const DB::Type::TableProperty &property);
    static void drop_table(const std::string &table_name);
    void insert(std::initializer_list<std::string> args);
    void update(const std::string &col_name,
                const std::string &op,
                const std::string &value);
    void remove(const std::string &col_name,
                const std::string &op,
                const std::string &value);

private:
    DB::Type::TableProperty property;

    void read_meta_data(const std::string &table_name);
    static void write_meta_data(const DB::Type::TableProperty &property);
    // function
//    template <typename T>
//    static DB::Type::Bytes make_tuple(const T &t);
};

//template <typename T>
//DB::Type::Bytes Table::make_tuple(const T &t){
//    DB::Type::Bytes tuple(sizeof(t));
//    std::memcpy(tuple.data(), &t, sizeof(t));
//    return tuple;
//}
//
//template <typename T, typename ...Args>
//DB::Type::Bytes Table::make_tuple(const T &t, const Args& ...args){
//    DB::Type::Bytes tuple(sizeof(t));
//    std::memcpy(tuple.data(), &t, sizeof(t));
//    DB::Type::Bytes bytes = make_tuple(args...);
//    tuple.insert(tuple.end(), bytes.begin(), bytes.end());
//    return tuple;
//}

#endif
