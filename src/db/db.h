#ifndef DB_H
#define DB_H

#include <string>
#include <unordered_set>
#include "util.h"
#include "table.h"

class DB {
public:
    // type
    using TableNameSet = std::unordered_set<std::string>;
    using Value = SDB::Type::Value;
    using Tuple = SDB::Type::Tuple;
    using TupleLst = SDB::Type::TupleLst;

public:
    DB(const std::string &db_name):db_name(db_name){
        read_meta_data();
    }

    static void create_db(const std::string &db_name);
    void drop_db();

    void create_table(const SDB::Type::TableProperty &table_property);
    void drop_table(const std::string &table_name);
    void insert(const std::string &table_name, const Tuple &tuple);
    void remove(const std::string &table_name, const std::string &col_name, const Value &value);
    void remove(const std::string &table_name, const std::string &col_name, SDB::Type::BVFunc predicate);
    void update(const std::string &table_name, 
                const std::string &pred_col_name, SDB::Type::BVFunc predicate,
                const std::string &op_col_name, SDB::Type::VVFunc op);
    DB::TupleLst find(const std::string &table_name,
                      const std::string &col_name,
                      const SDB::Type::Value &value);
    DB::TupleLst find(const std::string &table_name,
                      const std::string &col_name,
                      std::function<bool(DB::Value)> predicate);

private:
    static void write_meta_data(const std::string &db_name, const TableNameSet &set);
    void read_meta_data();

    static std::string get_meta_path(const std::string &db_name);

    // check integrity
    template <typename T>
    void check_referenced(const Table &table, T t);
    void check_referencing(const Table &table, const Tuple &tuple);


private:
    std::string db_name;
    TableNameSet table_name_set;
};


#endif //DB_H
