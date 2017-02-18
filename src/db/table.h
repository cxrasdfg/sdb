#ifndef DBIO_H
#define DBIO_H

#include <vector>
#include <string>
#include "record.h"

class Table {
public:
    Table()= delete;
//    Table(const Table &table)
//            :table_name(table.table_name), block_size(table.block_size){}
//    Table(Table &&table)
//            :table_name(table.table_name), block_size(table.block_size){}
    Table(const std::string &str, size_t size)
            :table_name(str), block_size(size){}
//
    std::string get_file_abs_path(bool is_index);

    // write
    void write_block(const std::vector<std::string> &data_block, int block_num, bool is_index);
    void write_record(const Record &record, int block_num);

    //read
    std::vector<std::string> read_block(int block_num, bool is_index);
    Record read_record(int block_num);

    // getter setter
    const std::string &get_table_name() const;
    void set_table_name(const std::string &table_name);
    size_t get_block_size() const;
    void set_block_size(size_t block_size);

private:
    std::string table_name;
    size_t block_size;
};

#endif //DBIO_H
