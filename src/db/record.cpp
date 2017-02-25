#include "record.h"
#include "io.h"
#include "util.h"
#include <map>

using DB::Type::Pos;
using DB::Type::Bytes ;
using DB::Type::BytesList ;
using DB::Const::BLOCK_SIZE;

DB::Type::BytesList Record::read_record(const DB::Type::PosList &pos_lst) {
    std::map<size_t, std::vector<Pos>> block_offsets_map;
    for (auto &&item : pos_lst) {
        size_t block_num = item / BLOCK_SIZE;
        Pos offset = item % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(offset);
    }
    IO io(property.get_file_abs_path(DB::Enum::RECORD_SUFFIX));
    size_t record_size = property.get_record_size();
    BytesList bytes_list;
    for (auto &&item: block_offsets_map) {
        DB::Type::Bytes data_block = io.read_block(item.first);
        for (auto &&offset : item.second) {
            auto beg = data_block.data()+offset;
            DB::Type::Bytes record_tuple(beg, beg+record_size);
            bytes_list.push_back(record_tuple);
        }
    }
    return bytes_list;
}

DB::Type::Pos Record::insert_record(const DB::Type::Bytes &data) {
    if (data.size() != property.get_record_size()) {
        throw std::runtime_error("Error: record_tuple_lst is empty");
    }
    Pos pos;
    if (free_pos_lst.empty()) {
        pos = free_end_pos;
    } else {
        pos = free_pos_lst.back();
        free_pos_lst.pop_back();
    }
    size_t block_num = pos / BLOCK_SIZE;
    Pos offset =  pos % BLOCK_SIZE;
    IO io(property.get_file_abs_path(DB::Enum::RECORD_SUFFIX));
    Bytes block_data = io.read_block(block_num);
    std::memcpy(block_data.data()+offset, data.data(), data.size());
    io.write_block(block_data, block_num);
    return pos;
}

void Record::remove_record(DB::Type::Pos pos) {
    if (pos > free_end_pos) {
        throw std::runtime_error(
                std::string("Error: pos error:") + std::to_string(pos)
        );
    }
    free_pos_lst.push_back(pos);
}


void Record::read_free_pos() {
    IO io(property.get_file_abs_path(DB::Enum::POS_SUFFIX));
    DB::Type::Bytes block_data = io.read_block(0);
    size_t pos_count;
    size_t pos_len = sizeof(pos_count);
    std::memcpy(&pos_count, block_data.data(), pos_len);
    auto beg = block_data.data()+pos_len;
    for (size_t i = 0; i < pos_count; ++i) {
        Pos pos;
        std::memcpy(&pos, beg+(pos_len*i), pos_len);
        free_pos_lst.push_back(pos);
    }
    free_end_pos = free_pos_lst.back();
    free_pos_lst.pop_back();
}
