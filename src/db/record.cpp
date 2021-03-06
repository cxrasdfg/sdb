#include <map>
#include <functional>

#include "record.h"
#include "util.h"
#include "cache.h"
#include "io.h"

using SDB::Type::Pos;
using SDB::Type::Bytes ;
using SDB::Type::BytesList ;
using SDB::Const::BLOCK_SIZE;
using SDB::Const::SIZE_SIZE;
using SDB::Const::POS_SIZE;

using namespace SDB;

Record::TupleLst Record::read_record(const SDB::Type::PosList &pos_lst) {
    std::map<size_t, std::vector<size_t>> block_offsets_map;
    for (auto &&item : pos_lst) {
        size_t block_num = item / BLOCK_SIZE;
        size_t offset = item % BLOCK_SIZE;
        block_offsets_map[block_num].push_back(offset);
    }
    TupleLst tuple_lst(property.tuple_property);
    for (auto &&item: block_offsets_map) {
        SDB::Type::Bytes data_block = Cache::make().read_block(get_record_path(property), item.first);
        for (auto offset : item.second) {
            Tuple tuple = bytes_to_tuple(property, data_block.data(), offset);
            tuple_lst.tuple_lst.push_back(tuple);
        }
    }
    return tuple_lst;
}

Record::TupleLst Record::read_record(size_t block_num) {
    TupleLst tuple_lst(property.tuple_property);
    if (block_num > end_block_num) {
        if (block_num-end_block_num==1 && free_pos_lst.empty()) {
            return tuple_lst;
        }
        throw std::runtime_error("Error: [read_record] block error");
    }
    Bytes bytes = Cache::make().read_block(get_record_path(property), block_num);
    size_t offset = 0;
    while (offset != BLOCK_SIZE) {
        if (offset > BLOCK_SIZE) {
            throw std::runtime_error("Error: [read_record] offset error");
        }
        auto iter = free_pos_lst.find(block_num*BLOCK_SIZE+offset);
        if (iter == free_pos_lst.end()) {
            Tuple tuple = bytes_to_tuple(property, bytes.data(), offset);
            tuple_lst.tuple_lst.push_back(tuple);
        } else {
            offset += iter->second;
        }
    }
    return tuple_lst;
}

void Record::write_record(size_t block_num, const TupleLst &tuple_lst) {
    if (block_num > end_block_num) {
        throw std::runtime_error("Error: [write_record] block error");
    }
    size_t offset = 0;
    Bytes block_data(BLOCK_SIZE);
    for (auto &&tuple : tuple_lst.tuple_lst) {
        if (offset > BLOCK_SIZE) {
            throw std::runtime_error("Error: [write_record] offset error");
        }
        auto iter = free_pos_lst.find(block_num*BLOCK_SIZE+offset);
        if (iter != free_pos_lst.end()) {
            offset += iter->second;
        } else {
            Bytes bytes = tuple_to_bytes(tuple);
            std::memcpy(block_data.data()+offset, bytes.data(), bytes.size());
            offset += bytes.size();
        }
    }
    auto iter = free_pos_lst.find(block_num*BLOCK_SIZE+offset);
    if (iter != free_pos_lst.end()) {
        offset += iter->second;
    }
    if (offset != BLOCK_SIZE) {
        throw std::runtime_error("Error: [write_record] offset error");
    }
    Cache::make().write_block(get_record_path(property), block_num, block_data);
}

void Record::update(const std::string &pred_col_name, SDB::Type::BVFunc bvFunc,
                    const std::string &op_col_name, SDB::Type::VVFunc vvFunc) {
    size_t block_num = 0;
    while (block_num > end_block_num) {
        TupleLst tuple_lst = read_record(block_num);
        for (auto &&tuple : tuple_lst.tuple_lst) {
            tuple.set_col_value(property.tuple_property, pred_col_name, bvFunc, op_col_name, vvFunc);
        }
        write_record(block_num, tuple_lst);
        block_num++;
    }
}

Pos Record::update(Pos pos, const Bytes &data) {
    remove_record(pos);
    return insert_record(data);
}

SDB::Type::Pos Record::insert_record(const SDB::Type::Bytes &data) {
    if (data.size() > BLOCK_SIZE) {
        throw std::runtime_error("Error: data size greater BLOCK_SIZE");
    }
    size_t pos = get_free_pos(data.size());
    size_t block_num = pos / BLOCK_SIZE;
    size_t offset = pos % BLOCK_SIZE;
    Cache &cache = Cache::make();
    Bytes block_data = cache.read_block(get_record_path(property), block_num);
    std::memcpy(block_data.data()+offset, data.data(), data.size());
    cache.write_block(get_record_path(property), block_num, block_data);
    return pos;
}

void Record::remove_record(SDB::Type::Pos pos) {
    if (free_pos_lst.find(pos) != free_pos_lst.end()) {
        return;
    }
    size_t block_num = pos / BLOCK_SIZE;
    Bytes bytes = Cache::make().read_block(get_record_path(property), block_num);
    size_t start = pos % BLOCK_SIZE;
    size_t offset = pos % BLOCK_SIZE;
    bytes_to_tuple(property, bytes.data(), offset);
    free_pos_lst[pos] = offset - start;
}

Record::TupleLst Record::find(const std::string &col_name, std::function<bool(Value)> predicate) {
    TupleLst tuple_lst(property.tuple_property);
    for (size_t i = 0; i <= end_block_num; ++i) {
        TupleLst block_tuple_lst = read_record(i);
        for (auto &&tuple: block_tuple_lst.tuple_lst) {
            Value value = tuple.get_col_value(property.tuple_property, col_name);
            if (predicate(value)) {
                tuple_lst.tuple_lst.push_back(tuple);
            }
        }
    }
    return tuple_lst;
}

// - get -
Pos Record::get_free_pos(size_t data_size) {
    size_t pos;
    for (auto &&item : free_pos_lst) {
        if (item.second == data_size) {
            pos = item.first;
            free_pos_lst.erase(item.first);
            return pos;
        } else if (item.second > data_size) {
            pos = item.first;
            size_t size = item.second - data_size;
            free_pos_lst.erase(item.first);
            free_pos_lst[pos+data_size] = size;
            return pos;
        }
    }
    end_block_num++;
    free_pos_lst[end_block_num*BLOCK_SIZE+data_size] = BLOCK_SIZE-data_size;
    return end_block_num*BLOCK_SIZE;
}

// tuple
SDB::Type::Bytes Record::tuple_to_bytes(const Tuple &tuple) {
    Bytes bytes;
    for (auto &&value : tuple.value_lst) {
        Bytes value_bytes = value_to_bytes(value);
        bytes.insert(bytes.end(), value_bytes.begin(), value_bytes.end());
    }
    return bytes;
}

Record::Tuple Record::bytes_to_tuple(const TableProperty &property,
                                     const Byte *base,
                                     size_t &offset) {
    Tuple tuple;
    for (auto &&item : property.tuple_property.property_lst) {
        Value value = bytes_to_value(item.col_type, base, offset);
        tuple.value_lst.push_back(value);
    }
    return tuple;
}

Record::Value Record::bytes_to_value(SDB::Enum::ColType type,
                                     const Byte *base,
                                     size_t &offset) {
    Bytes bytes;
    size_t len;
    if (Value::is_var_type(type)) {
        std::memcpy(&len, base+offset, SDB::Const::SIZE_SIZE);
        offset += SDB::Const::SIZE_SIZE;
    } else {
        len = SDB::Function::get_type_len(type);
    }
    bytes.insert(bytes.end(), base+offset, base+offset+len);
    offset += len;
    return Value::make(type, bytes);
}

Bytes Record::value_to_bytes(const Value &value) {
    Bytes bytes;
    if (value.is_var_type()) {
        Function::bytes_append(bytes, value.data.size());
    }
    bytes.insert(bytes.end(), value.data.begin(), value.data.end());
    return bytes;
}

void Record::create(const TableProperty &property) {
    // meta_record
    Bytes record_bytes;
    Bytes pos_count_bytes = SDB::Function::en_bytes(size_t(1));
    record_bytes.insert(record_bytes.end(), pos_count_bytes.begin(), pos_count_bytes.end());
    Bytes pos_bytes = SDB::Function::en_bytes(SDB::Type::Pos(0));
    record_bytes.insert(record_bytes.end(), pos_bytes.begin(), pos_bytes.end());
    Bytes size_bytes = SDB::Function::en_bytes(size_t(BLOCK_SIZE));
    record_bytes.insert(record_bytes.end(), size_bytes.begin(), size_bytes.end());
    Bytes end_block_bytes = SDB::Function::en_bytes(size_t(0));
    record_bytes.insert(record_bytes.end(), end_block_bytes.begin(), end_block_bytes.end());
    Cache::make().write_file(get_record_meta_path(property), record_bytes);

    // record
    IO::create_file(get_record_path(property));
}

void Record::drop(const TableProperty &property) {
    IO::delete_file(get_record_path(property));
    IO::delete_file(get_record_meta_path(property));
}

// ========== private =========
void Record::read_meta_data() {
    SDB::Type::Bytes block_data = Cache::make().read_file(get_record_meta_path(property));
    size_t pos_count;
    std::memcpy(&pos_count, block_data.data(), SIZE_SIZE);
    auto beg = block_data.data()+SIZE_SIZE;
    for (size_t i = 0; i < pos_count; ++i) {
        Pos pos;
        size_t size;
        std::memcpy(&pos, beg+((POS_SIZE+SIZE_SIZE)*i), POS_SIZE);
        std::memcpy(&size, beg+((POS_SIZE+SIZE_SIZE)*i)+POS_SIZE, SIZE_SIZE);
//        free_pos_lst.insert(pos, size);
        free_pos_lst[pos] = size;
    }
    std::memcpy(&end_block_num, beg+(pos_count*(POS_SIZE+SIZE_SIZE)), SIZE_SIZE);
}

void Record::write_meta_data() {
    SDB::Type::Bytes bytes;
    size_t lst_len = free_pos_lst.size();
    Bytes lst_len_bytes = SDB::Function::en_bytes(lst_len);
    bytes.insert(bytes.end(), lst_len_bytes.begin(), lst_len_bytes.end());
    for (auto &&item : free_pos_lst) {
        Bytes pos_bytes = SDB::Function::en_bytes(item.first);
        Bytes size_bytes = SDB::Function::en_bytes(item.second);
        bytes.insert(bytes.end(), pos_bytes.begin(), pos_bytes.end());
        bytes.insert(bytes.end(), size_bytes.begin(), size_bytes.end());
    }
    Bytes block_num = SDB::Function::en_bytes(end_block_num);
    bytes.insert(bytes.end(), block_num.begin(), block_num.end());
    Cache::make().write_file(get_record_meta_path(property), bytes);
}

std::string Record::get_record_path(const TableProperty &property) {
    return property.db_name + "/" + property.table_name+"/record.sdb";
}

std::string Record::get_record_meta_path(const TableProperty &property) {
    return property.db_name + "/" + property.table_name+"/record_meta.sdb";
}
