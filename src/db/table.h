#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>

#include "record.h"
#include "util.h"
#include "table_property.h"

class Table {
public:
    Table()= delete;
//    Table(const Table &table)
//            :table_name(table.table_name), BLOCK_SIZE(table.BLOCK_SIZE){}
//    Table(Table &&table)
//            :table_name(table.table_name), BLOCK_SIZE(table.BLOCK_SIZE){}
    Table(const TableProperty &property):property(property){}

    // get
    std::string get_file_abs_path(bool is_index)const;

    //read
    RecordList read_record(const std::vector<DB::Type::Pos> &pos_lst)const;

    // write
    void write_record(const RecordList &record_lst,
                      const std::vector<DB::Type::Pos> &pos_lst);

private:
    TableProperty property;
};

#endif
