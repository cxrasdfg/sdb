#include "record.h"
#include "io.h"
#include "util.h"
#include <map>
#include <functional>

using DB::Type::size_t;
using DB::Type::Bytes ;
using DB::Type::BytesList ;
using DB::Const::BLOCK_SIZE;

DB::Type::BytesList Record::read_record(const DB::Type::PosList &pos_lst) {
    std::map<size_t, std::vector<size_t>> block_offsets_map;
    for (auto &&item : pos_lst) {
        size_t block_num = item / BLOCK_SIZE;
        size_t offset = item % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(offset);
    }
    IO io(property.table_name+"_record.sdb");
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

DB::Type::size_t Record::insert_record(const DB::Type::Bytes &data) {
    if (data.size() != property.get_record_size()) {
        throw std::runtime_error("Error: record_tuple_lst is empty");
    }
    size_t pos;
    if (free_pos_lst.empty()) {
        pos = free_end_pos;
        free_end_pos += property.get_record_size();
    } else {
        pos = free_pos_lst.back();
        free_pos_lst.pop_back();
    }
    size_t block_num = pos / BLOCK_SIZE;
    size_t offset =  pos % BLOCK_SIZE;
    IO io(property.table_name+"_record.sdb");
    Bytes block_data = io.read_block(block_num);
    std::memcpy(block_data.data()+offset, data.data(), data.size());
    io.write_block(block_data, block_num);
    return pos;
}

void Record::remove_record(DB::Type::size_t pos) {
    if (pos > free_end_pos) {
        throw std::runtime_error(
                std::string("Error: Record pos error:") + std::to_string(pos)
        );
    }
    free_pos_lst.push_back(pos);
}


void Record::read_free_pos() {
    IO io(property.table_name+"_meta_record.sdb");
    DB::Type::Bytes block_data = io.read_file();
    size_t pos_count;
    size_t pos_len = sizeof(pos_count);
    std::memcpy(&pos_count, block_data.data(), pos_len);
    auto beg = block_data.data()+pos_len;
    for (size_t i = 0; i < pos_count; ++i) {
        size_t pos;
        std::memcpy(&pos, beg+(pos_len*i), pos_len);
        free_pos_lst.push_back(pos);
    }
    std::memcpy(&free_end_pos, beg+(pos_count*pos_len), pos_len);
}

void Record::write_free_pos() {
    size_t size_len = sizeof(size_t);
    size_t Pos_len = sizeof(size_t);
    DB::Type::Bytes bytes(size_len*2+free_pos_lst.size()*Pos_len);
    size_t free_pos_lst_len = free_pos_lst.size();
    std::memcpy(bytes.data(), &free_pos_lst_len, size_len);
    auto beg = bytes.data()+sizeof(size_t);
    for (size_t i = 0; i < free_pos_lst_len; ++i) {
        std::memcpy(beg+(i*Pos_len), &free_pos_lst[i], Pos_len);
    }
    std::memcpy(beg+(Pos_len*free_pos_lst_len), &free_end_pos, Pos_len);
    IO io(property.table_name+"_meta_record.sdb");
    io.write_file(bytes);
}
