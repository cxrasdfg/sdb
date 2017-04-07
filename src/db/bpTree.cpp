#include <memory>
#include <list>
#include <stdexcept>
#include <iostream>
#include <string>
#include <boost/variant.hpp>

#include "util.h"
#include "record.h"
#include "bpTree.h"
#include "cache.h"
#include "io.h"

using SDB::Const::BLOCK_SIZE;
using SDB::Type::Bytes;
using SDB::Type::Pos;
using SDB::Type::PosList;
using SDB::Type::Value;

using namespace SDB;


// --------------- Function ---------------
// ========== BptNode Function =========
Value BptNode::last_key()const{
    return (std::prev(pos_lst.end()))->first;
}

// ========== BpTree Function =========
// ---------- BpTree Public Function ---------
BpTree::BpTree(const SDB::Type::TableProperty &table_property):table_property(table_property){
    initialize();
}

BpTree::~BpTree(){
    write_info_block();
    clear();
}

void BpTree::clear() {
}

void BpTree::write_info_block() {
    Bytes data(BLOCK_SIZE);
    // root pos
    size_t Pos_len = sizeof(root_pos);
    std::memcpy(data.data(), &root_pos, Pos_len);
    // free pos list
    size_t size_len = sizeof(size_t);
    size_t free_pos_count = free_pos_list.size();
    std::memcpy(data.data()+Pos_len, &free_pos_count, size_len);
    auto beg = data.data() + Pos_len + size_len;
    for (size_t j = 0; j < free_pos_count; ++j) {
        std::memcpy(beg+(j*Pos_len), &free_pos_list[j], Pos_len);
    }
    std::memcpy(beg+(free_pos_count*Pos_len), &free_end_pos, Pos_len);
    Cache::make().write_file(get_index_meta_path(table_property), data);
}

void BpTree::initialize() {
    // set node_key_count
    size_t size_len = sizeof(size_t);
    size_t key_size = table_property.tuple_property.get_type_size(table_property.key);
    size_t Pos_len = sizeof(size_t);
    node_key_count = (SDB::Const::BLOCK_SIZE-Pos_len-1-size_len)/(key_size+Pos_len);
    // read info block
    Bytes block_data = Cache::make().read_file(get_index_meta_path(table_property));
    // set root_pos
    std::memcpy(&root_pos, block_data.data(), Pos_len);
    // get free pos
    size_t free_pos_count;
    std::memcpy(&free_pos_count, block_data.data()+Pos_len, sizeof(size_t));
    auto beg = block_data.data()+Pos_len+size_len;
    for (size_t j = 0; j < free_pos_count; ++j) {
        Pos pos;
        std::memcpy(&pos, beg+(j*Pos_len), Pos_len);
        free_pos_list.push_back(pos);
    }
    std::memcpy(&free_end_pos, beg+(free_pos_count*Pos_len), Pos_len);
}

void BpTree::insert(const Value &key, const Bytes &data){
    auto root = read(root_pos);
    if (!root) {
        nodePtrType new_root;
        Record record_list(table_property);
        auto record_pos = record_list.insert_record(data);
        new_root = std::make_shared<BptNode>();
        new_root->pos_lst.push_back(std::make_pair(key, record_pos));
        new_root->is_leaf = true;
        write(new_root);
        root_pos = new_root->file_pos;
        return;
    }
    nodePtrType right_ptr = insert_r(key, data, root);
    if (right_ptr){
        auto new_root = std::make_shared<BptNode>();
        new_root->pos_lst.push_back(std::make_pair(root->last_key(), root->file_pos));
        new_root->pos_lst.push_back(std::make_pair(right_ptr->last_key(), right_ptr->file_pos));
        new_root->is_leaf = false;
        write(new_root);
        root_pos = new_root->file_pos;
    }
}

void BpTree::remove(const Value &key){
    nodePtrType root = read(root_pos);
    if (root == nullptr){
        throw_error("Error: B+ Tree is empty");
    }
    remove_r(key, root);
    if (!root->is_leaf && root->pos_lst.size() == 1){
        root_pos = read(root->pos_lst.begin()->second)->file_pos;
        free_pos_list.push_back(root->file_pos);
    } else if (root->pos_lst.size() == 0) {
        free_end_pos = 0;
        free_pos_list.clear();
        root_pos = 0;
    }
}

void BpTree::update(const Value &key, const Bytes &data) {
    nodePtrType ptr = read(root_pos);
    if (ptr == nullptr){
        throw_error("Error: bpTree is empty!");
    }
    while (ptr != nullptr) {
        bool is_leaf = ptr->is_leaf;
        bool is_for_end = true;
        for (auto &&x: ptr->pos_lst) {
            if (key == x.first && is_leaf){
                Record record(table_property);
                x.second = record.update(x.second, data);
                write(ptr);
                return;
            } else if (!is_leaf && key <= x.first){
                ptr = read(x.second);
                is_for_end = false;
                break;
            } else if (key < x.first && is_leaf) {
                break;
            }
        }
        if (is_for_end) {
            throw_error("Error: can't fount key");
        }
    }
}

BpTree::nodePtrType BpTree::read(SDB::Type::Pos pos) const{
    if (free_end_pos == 0) {
        return nullptr;
    }
    Bytes block_data = Cache::make().read_block(get_index_path(table_property), pos / BLOCK_SIZE);

    // ptr
    nodePtrType ptr = std::make_shared<BptNode>();

    // leaf
    ptr->is_leaf = block_data[0];

    // get pos list
    size_t key_len = table_property.tuple_property.get_type_size(table_property.key);
    size_t item_len = key_len + sizeof(pos);
    size_t pos_lst_len;
    std::memcpy(&pos_lst_len, block_data.data()+1, sizeof(size_t));
    auto beg = block_data.data()+1+sizeof(size_t);
    for (size_t i = 0; i < pos_lst_len; ++i) {
        Bytes item_bytes(beg+(i*item_len), beg+((i+1)*item_len));
        auto item_beg = beg+(i*item_len);
        auto item_tem = item_beg+key_len;
        auto item_end = item_tem + sizeof(pos);
        auto type = table_property.tuple_property.get_col_type(table_property.key);
        Bytes key_data(item_beg, item_beg+key_len);
        Value key(type, key_data);
        Pos child_pos;
        std::memcpy(&child_pos, item_tem, sizeof(pos));
        auto item = std::make_pair(key, child_pos);
        ptr->pos_lst.push_back(item);
    }
    if (ptr->is_leaf) {
        std::memcpy(&ptr->end_pos, beg+(pos_lst_len*item_len), sizeof(ptr->end_pos));
    }
    ptr->file_pos = pos;
    ptr->is_new_node = false;
    return ptr;
}

void BpTree::write(nodePtrType ptr) {
    using SDB::Function::en_bytes;
    Bytes block_data(SDB::Const::BLOCK_SIZE);
    size_t key_len = table_property.tuple_property.get_type_size(table_property.key);
    size_t item_len = key_len + sizeof(size_t);
    size_t size_len = sizeof(size_t);
    size_t char_len = sizeof(char);
    char is_leaf = ptr->is_leaf;
    std::memcpy(block_data.data(), &is_leaf, char_len);
    size_t pos_lst_len = ptr->pos_lst.size();
    std::memcpy(block_data.data()+char_len, &pos_lst_len, size_len);
    auto beg = block_data.data()+sizeof(char)+size_len;
    size_t offset = 0;
    for (auto &&item : ptr->pos_lst) {
        Bytes key_bytes = item.first.data;
        std::memcpy(beg+offset, key_bytes.data(), key_len);
        std::memcpy(beg+offset+key_len, &item.second, sizeof(size_t));
        offset += item_len;
    }
    if (is_leaf) {
        std::memcpy(beg+offset, &ptr->end_pos, key_len);
    }
    Pos write_pos;
    if (!ptr->is_new_node) {
        write_pos = ptr->file_pos;
    } else if (free_pos_list.empty()) {
        write_pos = free_end_pos;
        free_end_pos += BLOCK_SIZE;
    } else {
        write_pos = free_pos_list.back();
        free_pos_list.pop_back();
    }
    ptr->file_pos = write_pos;
    size_t block_num = write_pos / BLOCK_SIZE;
    Cache::make().write_block(get_index_path(table_property), block_num, block_data);
}

void BpTree::create(const TableProperty &property) {
    // meta_index.sdb
    using Const::SIZE_SIZE;
    Bytes bytes(Const::POS_SIZE+Const::SIZE_SIZE+Const::SIZE_SIZE);
    size_t root_pos = 0;
    size_t free_pos_count = 0;
    size_t free_end_pos = 0;
    std::memcpy(bytes.data(), &root_pos, Const::POS_SIZE);
    std::memcpy(bytes.data()+SIZE_SIZE, &free_pos_count, SIZE_SIZE);
    std::memcpy(bytes.data()+SIZE_SIZE+SIZE_SIZE, &free_end_pos, SIZE_SIZE);
    Cache::make().write_file(get_index_meta_path(property), bytes);
    // index
    IO::create_file(get_index_path(property));
}

void BpTree::drop(const TableProperty &property) {
    IO::delete_file(get_index_path(property));
    IO::delete_file(get_index_meta_path(property));
}

void BpTree::print()const{
    nodePtrType root_node = read(root_pos);
    if (root_node == nullptr) {
        return;
    }
    std::list<nodePtrType> deq;
    deq.push_back(root_node);
    size_t sub_count = 1;
    size_t level_f = 0;
    while (!deq.empty()) {
        nodePtrType ptr = deq.front();
        deq.pop_front();
        sub_count--;
        if (ptr == nullptr){
            std::cout << "nullptr";
            continue;
        }
        bool is_leaf = ptr->is_leaf;
        std::cout << "[ ";
        for (auto iter = ptr->pos_lst.begin(); iter != ptr->pos_lst.end(); iter++){
            if (is_leaf){
                std::cout << iter->first.get_string() << ":" << iter->second << " ";
//                std::cout << iter->first << ":";
            } else {
                std::cout << iter->first.get_string() << " ";
                auto node = read(iter->second);
                deq.push_back(node);
            }
        }
        std::cout << "]";
        if (sub_count == 0){
            sub_count = deq.size();
            std::cout << std::endl;
            level_f++;
        }
    }
}

// ---------- BpTree Private Function ---------
BpTree::nodePtrType BpTree::node_split(nodePtrType &ptr) {
    auto right_node_ptr = std::make_shared<BptNode>();
    auto &right_lst = right_node_ptr->pos_lst;
    right_lst.splice(right_lst.end(), ptr->pos_lst,
                     std::next(ptr->pos_lst.begin(), ptr->pos_lst.size()/2), ptr->pos_lst.end());
    right_node_ptr->is_leaf = ptr->is_leaf;
    right_node_ptr->end_pos = ptr->end_pos;
    write(right_node_ptr);
    if (ptr->is_leaf){
        ptr->end_pos = right_node_ptr->file_pos;
    }
    write(ptr);
    return right_node_ptr;
}

bool BpTree::node_merge(nodePtrType &ptr_1, nodePtrType &ptr_2) {
    ptr_1->pos_lst.splice(ptr_1->pos_lst.end(), ptr_2->pos_lst);
    if (ptr_1->pos_lst.size() > node_key_count){
        auto right_ptr = node_split(ptr_1);
        free_pos_list.push_back(ptr_2->file_pos);
        right_ptr->end_pos = ptr_2->end_pos;
        ptr_2 = right_ptr;
        write(ptr_1);
        write(ptr_2);
        return false;
    } else {
        ptr_1->end_pos = ptr_2->end_pos;
        write(ptr_1);
        free_pos_list.push_back(ptr_2->file_pos);
        ptr_2 = nullptr;
        return true;
    }
}

bool BpTree::is_node_less(nodePtrType ptr)const{
    return (node_key_count/2) > ptr->pos_lst.size();
}

// ========== Query ===========
BpTree::PosList BpTree::find(const Value &key) const{
    nodePtrType ptr = read(root_pos);
    if (ptr == nullptr){
        throw_error("Error: bpTree is empty!");
    }
    while (ptr != nullptr) {
        bool is_leaf = ptr->is_leaf;
        bool is_for_end = true;
        for (auto &&x: ptr->pos_lst) {
            if (key == x.first && is_leaf){
                PosList pos_lst;
                pos_lst.push_back(x.second);
                return pos_lst;
            } else if (!is_leaf && key <= x.first){
                ptr = read(x.second);
                is_for_end = false;
                break;
            } else if (key < x.first && is_leaf) {
                break;
            }
        }
        if (is_for_end) {
            return PosList();
        }
    }
}

PosList BpTree::find(const Value &beg, const Value &end) const {
    PosList pos_lst;
    nodePtrType beg_ptr = find_near_key_node(beg);
    nodePtrType end_ptr = find_near_key_node(end);
    if (beg_ptr->file_pos == end_ptr->file_pos) {
        auto beg_iter = get_pos_lst_iter(beg, beg_ptr->pos_lst);
        auto end_iter = get_pos_lst_iter(end, beg_ptr->pos_lst);
        pos_lst_insert(pos_lst, beg_iter, end_iter);
        return pos_lst;
    }
    auto beg_iter = get_pos_lst_iter(beg, beg_ptr->pos_lst);
    auto end_iter = get_pos_lst_iter(end, end_ptr->pos_lst);
    pos_lst_insert(pos_lst, beg_iter, beg_ptr->pos_lst.end());
    pos_lst_insert(pos_lst, end_ptr->pos_lst.begin(), end_iter);
    nodePtrType ptr = read(beg_ptr->end_pos);
    while (ptr->file_pos != end_ptr->file_pos) {
        pos_lst_insert(pos_lst, ptr->pos_lst.begin(), ptr->pos_lst.end());
        ptr = read(ptr->end_pos);
    }
    return pos_lst;
}

PosList BpTree::find(const Value &mid, bool is_less) const {
    if (is_less) {
        return find(get_leaf_begin_node()->pos_lst.begin()->first, mid);
    } else {
        return find(mid, std::prev(get_leaf_end_node()->pos_lst.end())->first);
    }
}

PosList BpTree::find(std::function<bool(Value)> predicate) const {
    nodePtrType beg_ptr = get_leaf_begin_node();
    nodePtrType end_ptr = get_leaf_end_node();
    nodePtrType ptr = beg_ptr;
    PosList pos_lst;
    while (ptr->file_pos != end_ptr->file_pos) {
        pos_lst_insert(pos_lst, ptr->pos_lst.begin(), ptr->pos_lst.end(), predicate);
        ptr = read(ptr->end_pos);
    }
    pos_lst_insert(pos_lst, end_ptr->pos_lst.begin(), end_ptr->pos_lst.end(), predicate);
    return pos_lst;
}

BpTree::nodePtrType BpTree::insert_r(const Value &key, const Bytes &data, nodePtrType ptr) {
    bool is_leaf = ptr->is_leaf;
    bool is_for_end = true;
    for (auto iter=ptr->pos_lst.begin(); iter!=ptr->pos_lst.end(); iter++){
        if (iter->first == key){
            throw_error("Error: key already existed!");
        } else if (key < iter->first) {
            if (is_leaf){
                Record record(table_property);
                Pos record_pos = record.insert_record(data);
                ptr->pos_lst.insert(iter, std::make_pair(key, record_pos));
            } else {
                auto node = read(iter->second);
                auto right_ptr = insert_r(key, data, node);
                iter->first = node->last_key();
                if (right_ptr){
                    ptr->pos_lst.insert(std::next(iter), std::make_pair(right_ptr->last_key(), right_ptr->file_pos));
                }
            }
            is_for_end = false;
            break;
        }
    }
    if (is_for_end) {
        if (is_leaf){
            Record record(table_property);
            Pos record_pos = record.insert_record(data);
            ptr->pos_lst.insert(ptr->pos_lst.end(), std::make_pair(key, record_pos));
        } else {
            auto last_node_ptr = read(std::prev(ptr->pos_lst.end())->second);
            auto right_ptr = insert_r(key, data, last_node_ptr);
            std::prev(ptr->pos_lst.end())->first = last_node_ptr->last_key();
            if (right_ptr){
                ptr->pos_lst.insert(ptr->pos_lst.end(), std::make_pair(right_ptr->last_key(), right_ptr->file_pos));
            }
        }
    }
    write(ptr);
    return (ptr->pos_lst.size() > node_key_count) ? node_split(ptr) : nullptr;
}

bool BpTree::remove_r(const Value &key, nodePtrType &ptr) {
    bool is_leaf = ptr->is_leaf;
    bool is_for_end = true;
    for (auto iter = ptr->pos_lst.begin(); iter != ptr->pos_lst.end(); iter++){
        if (is_leaf) {
            if (key == iter->first) {
                Record record(table_property);
                record.remove_record(iter->second);
                ptr->pos_lst.erase(iter);
                is_for_end = false;
                break;
            } else if (key < iter->first) {
                throw_error("Error: can't find Key");
            }
        } else if (key <= iter->first) {
            nodePtrType iter_sec_ptr = read(iter->second);
            bool is_less = remove_r(key, iter_sec_ptr);
            iter->first = iter_sec_ptr->last_key();
            if (is_less){
                bool is_one;
                if (std::next(iter) == ptr->pos_lst.end()){
                    auto iter_prev = std::prev(iter);
                    nodePtrType prev_ptr = read(iter_prev->second);
                    is_one = node_merge(prev_ptr, iter_sec_ptr);
                    if (is_one) ptr->pos_lst.erase(iter);
                    iter_prev->first = read(iter_prev->second)->last_key();
                } else {
                    auto iter_next = std::next(iter);
                    nodePtrType next_ptr = read(iter_next->second);
                    is_one = node_merge(iter_sec_ptr, next_ptr);
                    if (is_one) {
                        ptr->pos_lst.erase(iter_next);
                    }
                    iter->first = iter_sec_ptr->last_key();
                }
            }
            is_for_end = false;
            break;
        }
    }
    if (is_for_end && is_leaf){
        throw_error("Error: can't find Key");
    }
    write(ptr);
    return is_node_less(ptr);
}

BpTree::nodePtrType BpTree::find_near_key_node(const Value &key)const {
    nodePtrType ptr = read(root_pos);
    if (ptr == nullptr){
        throw_error("Error: bpTree is empty!");
    }
    while (ptr != nullptr) {
        bool is_leaf = ptr->is_leaf;
        bool is_for_end = true;
        for (auto &&x: ptr->pos_lst) {
            if (is_leaf){
                return ptr;
            } else if (key <= x.first) {
                ptr = read(x.second);
                is_for_end = false;
                break;
            }
        }
        if (is_for_end){
            if (is_leaf) {
                return ptr;
            }
            ptr = read(std::prev(ptr->pos_lst.end())->second);
        }
    }
}

BpTree::nodePosLstType::const_iterator
BpTree::get_pos_lst_iter(const Value &key, const nodePosLstType &pos_lst) const {
    for (auto iter = pos_lst.cbegin(); iter != pos_lst.cend(); ++iter) {
        if (key <= iter->first) {
            return iter;
        }
    }
    return pos_lst.end();
}

BpTree::nodePtrType BpTree::get_leaf_begin_node()const{
    nodePtrType ptr = read(root_pos);
    if (ptr == nullptr) {
        throw std::runtime_error("Error: Empty bpTree!");
    }
    while (!ptr->is_leaf) {
        ptr = read(ptr->pos_lst.begin()->second);
    }
    return ptr;
}

BpTree::nodePtrType BpTree::get_leaf_end_node()const{
    nodePtrType ptr = read(root_pos);
    if (ptr == nullptr) {
        throw std::runtime_error("Error: Empty bpTree!");
    }
    while (!ptr->is_leaf) {
        Pos pos = std::prev(ptr->pos_lst.end())->second;
        ptr = read(pos);
    }
    return ptr;
}

void BpTree::pos_lst_insert(PosList &pos_lst,
                            nodePosLstType::const_iterator beg_iter,
                            nodePosLstType::const_iterator end_iter) const{
    for (auto it = beg_iter; it != end_iter; ++it) {
        pos_lst.push_back(it->second);
    }
}

void BpTree::pos_lst_insert(PosList &pos_lst,
                            nodePosLstType::const_iterator beg_iter,
                            nodePosLstType::const_iterator end_iter,
                            std::function<bool(Value)> predicate) const {
    for (auto it = beg_iter; it != end_iter; ++it) {
        if (predicate(it->first)) {
            pos_lst.push_back(it->second);
        }
    }
}

std::string BpTree::get_index_path(const TableProperty &property) {
    return property.db_name + "/" + property.table_name + "/index.sdb";
}

std::string BpTree::get_index_meta_path(const TableProperty &property) {
    return property.db_name + "/" + property.table_name + "/index_meta.sdb";
}
