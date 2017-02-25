#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>

#include "util.h"

class RecordList {
public:
    RecordList()= delete;
    RecordList(const DB::Type::TableProperty &property):property(property){}

    // record
    void read_record(const DB::Type::PosList &pos_lst);
    DB::Type::PosList write_record();

    void read_free_pos();

public:
    DB::Type::BytesList record_tuple_lst;
private:
    std::vector<DB::Type::Pos> free_pos_lst;
    DB::Type::Pos free_end_pos;
    DB::Type::TableProperty property;
};

#endif //MAIN_RECORD_H
