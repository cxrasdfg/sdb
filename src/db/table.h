#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>
#include <initializer_list>

#include "record.h"
#include "util.h"

class Table {
public:
    Table()= delete;
//    Table(const Table &table)
//            :table_name(table.table_name), BLOCK_SIZE(table.BLOCK_SIZE){}
//    Table(Table &&table)
//            :table_name(table.table_name), BLOCK_SIZE(table.BLOCK_SIZE){}
    Table(const DB::Type::TableProperty &property):property(property){}

    // get
    std::string get_file_abs_path(bool is_index)const;
    // tuple operate
    DB::Type::Bytes tuple_get_col_value(const DB::Type::Bytes &tuple,
                                        const std::string &col_name)const;

    // sql
    DB::Type::Bytes make_tuple(std::initializer_list<std::string> args);
//    template <typename T, typename ...Args>
//    static DB::Type::Bytes make_tuple(const T &t, const Args& ...args);
    void insert(std::initializer_list<std::string> args);
    void update(const std::string &col_name,
                const std::string &op,
                const std::string &value);
    void remove(const std::string &col_name,
                const std::string &op,
                const std::string &value);

private:
    DB::Type::TableProperty property;

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
