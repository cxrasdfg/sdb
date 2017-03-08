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
    DB::Type::size_t insert_record(const DB::Type::Bytes &data);
    void remove_record(DB::Type::size_t pos);

    // static method
    template <typename Func>
    static void tuple_op(const DB::Type::TableProperty &property,
                         const DB::Type::Bytes &bytes,
                         Func op);

private:
    void read_free_pos();
    void write_free_pos();

private:
    std::vector<DB::Type::size_t> free_pos_lst;
    DB::Type::size_t free_end_pos;
    DB::Type::TableProperty property;
};

template <typename Func>
void Record::tuple_op(const DB::Type::TableProperty &property,
                      const DB::Type::Bytes &bytes,
                      Func op) {
    using namespace DB::Enum;
    if (bytes.size() != property.get_record_size()) {
        throw std::runtime_error("Error: tuple print bytes size Error");
    }
    DB::Type::size_t offset = 0;
    for (auto &&item : property.col_property) {
        char col_type = item.second.first;
        size_t col_len = item.second.second;
        switch (col_type) {
            case CHAR:
                char ch;
                std::memcpy(&ch, bytes.data()+offset, sizeof(col_len));
                op(ch);
                break;
            case FLOAT:
                float fl;
                std::memcpy(&fl, bytes.data()+offset, sizeof(col_len));
                op(fl);
                break;
            case INT:
                int it;
                std::memcpy(&it, bytes.data()+offset, sizeof(col_len));
                op(it);
                break;
            case VARCHAR:
                op(std::string(bytes.data()+offset, bytes.data()+offset+col_len));
                break;
            default:
                throw std::runtime_error("Error: tuple op type error!");

        }
        offset += col_len;
    }
}
#endif //MAIN_RECORD_H
