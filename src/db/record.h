#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>
#include <functional>

#include "util.h"

class Record {
public:
    Record()= delete;
    Record(const DB::Type::TableProperty &property):property(property){
        read_free_pos();
    }
    ~Record(){
        write_free_pos();
    }

    // record
    DB::Type::BytesList read_record(const DB::Type::PosList &pos_lst);
    DB::Type::Pos insert_record(const DB::Type::Bytes &data);
    void remove_record(DB::Type::Pos pos);

private:
    void read_free_pos();
    void write_free_pos();

private:
    std::vector<DB::Type::Pos > free_pos_lst;
    DB::Type::Pos free_end_pos;
    DB::Type::TableProperty property;
};
#endif //MAIN_RECORD_H
