#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>
#include <functional>

#include "util.h"

class Record {
public:
    // type
    using Pos = DB::Type::Pos ;
    using PosList = DB::Type::PosList;
    using Byte = DB::Type::Byte;
    using Bytes = DB::Type::Bytes;
    using Value = DB::Type::Value;
    using Tuple = DB::Type::Tuple;
    using TupleLst = DB::Type::TupleLst;
    using TableProperty = DB::Type::TableProperty;

    Record()= delete;
    Record(const DB::Type::TableProperty &property):property(property){
        read_meta_data();
    }
    ~Record(){
        write_meta_data();
    }

    // record
    TupleLst read_record(const DB::Type::PosList &pos_lst);
    TupleLst read_record(size_t block_num);
    void write_record(size_t block_num, const TupleLst &tuple_lst);
    DB::Type::Pos insert_record(const DB::Type::Bytes &data);
    void remove_record(DB::Type::Pos pos);
    // only non-variable column data
    void update(const std::string &col_name, DB::Type::BVFunc bvFunc, DB::Type::VVFunc vvFunc);
    Pos update(Pos pos, const Bytes &data);
    TupleLst find(const std::string &col_name, std::function<bool(Value)> predicate);

    // get
    Pos get_free_pos(size_t data_size);

    // value_lst
    // convert
    static DB::Type::Bytes tuple_to_bytes(const Tuple &tuple);
    static Tuple bytes_to_tuple(const TableProperty &property, const Byte *base, size_t &offset);
    static Value bytes_to_value(DB::Enum::ColType type, const Byte *base, size_t &offset);
    static Bytes value_to_bytes(const Value &value);
private:
    void read_meta_data();
    void write_meta_data();

private:
    DB::Type::TableProperty property;
    std::map<DB::Type::Pos, size_t> free_pos_lst;
    size_t end_block_num;
    std::string record_path;
    std::string record_meta_path;
};
#endif //MAIN_RECORD_H
