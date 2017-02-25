#include "record.h"
#include "io.h"
#include "util.h"

using DB::Type::Pos;
using DB::Const::BLOCK_SIZE;

void RecordList::read_record(const DB::Type::PosList &pos_lst) {
    std::map<size_t, std::vector<Pos>> block_offsets_map;
    for (auto &&item : pos_lst) {
        size_t block_num = item / BLOCK_SIZE;
        Pos offset = item % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(offset);
    }
    IO io(property.get_file_abs_path(DB::Enum::RECORD_SUFFIX));
    size_t record_size = property.get_record_size();
    record_tuple_lst.clear();
    for (auto &&item: block_offsets_map) {
        DB::Type::Bytes data_block = io.read_block(item.first);
        for (auto &&offset : item.second) {
            auto beg = data_block.data()+offset;
            DB::Type::Bytes record_tuple(beg, beg+record_size);
            record_tuple_lst.push_back(record_tuple);
        }
    }
}

DB::Type::PosList RecordList::write_record() {
    if (record_tuple_lst.size() == 0) {
        throw std::runtime_error("Error: record_tuple_lst is empty");
    }
    std::map<size_t, std::vector<std::pair<Pos, DB::Type::Bytes>>> block_offsets_map;
    DB::Type::PosList pos_lst;
    if (record_tuple_lst.size() == 1 && !free_pos_lst.empty()) {
        auto free_pos = free_pos_lst.back();
        free_pos_lst.pop_back();
        size_t block_num = free_pos / BLOCK_SIZE;
        Pos offset =  free_pos % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(std::make_pair(offset, record_tuple_lst.back()));
        record_tuple_lst.pop_back();
        pos_lst.push_back(free_pos);
    }
    for (size_t i = 0; i < record_tuple_lst.size(); ++i) {
        size_t block_num = free_end_pos / BLOCK_SIZE;
        Pos offset =  free_end_pos % BLOCK_SIZE;
        if (record_tuple_lst.size() != property.get_record_size()) {
            throw std::runtime_error("Error: tuple length error");
        }
        block_offsets_map[block_num].push_back(std::make_pair(offset, record_tuple_lst[i]));
        pos_lst.push_back(free_end_pos);
        free_end_pos += property.get_record_size();
    }
    IO io(property.get_file_abs_path(DB::Enum::RECORD_SUFFIX));
    for (auto &&item : block_offsets_map) {
        DB::Type::Bytes block_buffer = io.read_block(item.first);
        for (auto &&second : item.second) {
            std::copy(second.second.begin(), second.second.end(), block_buffer.data()+second.first);
        }
        io.write_block(block_buffer, item.first);
    }
    return pos_lst;
}

void RecordList::read_free_pos() {
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
