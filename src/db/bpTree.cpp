#include <memory>
#include <list>
#include <stdexcept>
#include <iostream>
#include <string>
#include <boost/variant.hpp>

#include "util.h"
#include "io.h"
#include "record.h"
#include "bpTree.h"

using DB::Const::BLOCK_SIZE;
using DB::Type::Bytes;
using DB::Type::Pos;
using DB::Type::PosList;
using DB::Type::Value;


// --------------- Function ---------------
// ========== BptNode Function =========
Value BptNode::last_key()const{
    return (std::prev(pos_lst.end()))->first;
}

// ========== BpTree Function =========
// ---------- BpTree Public Function ---------
BpTree::BpTree(const DB::Type::TableProperty &table_property):table_property(table_property){
    initialize();
}

BpTree::~BpTree(){
    write_info_block();
    clear();
}

void BpTree::clear() {
}

void BpTree::write_info_block() {
    IO io(table_property.table_name+"_meta_index.sdb");
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
    io.write_file(data);
}

void BpTree::initialize() {
    // set node_key_count
    size_t size_len = sizeof(size_t);
    size_t key_size = table_property.col_property.at(table_property.key).second;
    size_t Pos_len = sizeof(size_t);
    node_key_count = (DB::Const::BLOCK_SIZE-Pos_len-1-size_len)/(key_size+Pos_len);
    // read info block
    IO io(table_property.table_name+"_meta_index.sdb");
    Bytes block_data = io.read_file();
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
    nodePtrType left_ptr = insert_r(key, data, root);
    if (left_ptr){
        write(left_ptr);
        auto new_root = std::make_shared<BptNode>();
        new_root->pos_lst.push_back(std::make_pair(left_ptr->last_key(), left_ptr->file_pos));
        new_root->pos_lst.push_back(std::make_pair(root->last_key(), root->file_pos));
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

BpTree::nodePtrType BpTree::read(DB::Type::Pos pos) const{
    if (free_end_pos == 0) {
        return nullptr;
    }
    IO io(table_property.table_name+"_index.sdb");
    Bytes block_data = io.read_block(pos / BLOCK_SIZE);

    // ptr
    nodePtrType ptr = std::make_shared<BptNode>();

    // leaf
    ptr->is_leaf = block_data[0];

    // get pos list
    size_t key_len = table_property.col_property.at(table_property.key).second;
    size_t item_len = key_len + sizeof(pos);
    size_t pos_lst_len;
    std::memcpy(&pos_lst_len, block_data.data()+1, sizeof(size_t));
    auto beg = block_data.data()+1+sizeof(size_t);
    for (size_t i = 0; i < pos_lst_len; ++i) {
        Bytes item_bytes(beg+(i*item_len), beg+((i+1)*item_len));
        auto item_beg = beg+(i*item_len);
        auto item_tem = item_beg+key_len;
        auto item_end = item_tem + sizeof(pos);
        auto type_info = table_property.col_property.at(table_property.key);
        Bytes key_data(item_beg, item_beg+key_len);
        Value key(type_info.first, type_info.second, key_data);
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
    using DB::Function::en_bytes;
    Bytes block_data(DB::Const::BLOCK_SIZE);
    size_t key_len = table_property.col_property.at(table_property.key).second;
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
        Bytes key_bytes = en_bytes(item.first);
        std::memcpy(beg+offset, key_bytes.data(), key_len);
        std::memcpy(beg+offset+key_len, &item.second, sizeof(size_t));
        offset += item_len;
    }
    if (is_leaf) {
        std::memcpy(beg+offset, &ptr->end_pos, key_len);
    }
    IO io(table_property.table_name+"_index.sdb");
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
    io.write_block(block_data, block_num);
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
    auto left_lst_ptr = std::make_shared<nodePosLstType>();
    left_lst_ptr->splice(left_lst_ptr->begin(), ptr->pos_lst,
                         ptr->pos_lst.begin(), std::next(ptr->pos_lst.begin(), ptr->pos_lst.size()/2));
    auto left_node_ptr = std::make_shared<BptNode>();
    left_node_ptr->pos_lst= *left_lst_ptr;
    if (ptr->is_leaf){
        left_node_ptr->end_pos = ptr->file_pos;
    }
    left_node_ptr->is_leaf = ptr->is_leaf;
    write(ptr);
    return left_node_ptr;
}

bool BpTree::node_merge(nodePtrType &ptr_1, nodePtrType &ptr_2) {
    ptr_2->pos_lst.splice(ptr_2->pos_lst.begin(), ptr_1->pos_lst);
    if (ptr_2->pos_lst.size() > node_key_count){
        auto left_ptr = node_split(ptr_2);
        left_ptr->file_pos = ptr_1->file_pos;
        left_ptr->is_new_node = false;
        ptr_1 = left_ptr;
        write(ptr_1);
        write(ptr_2);
        return false;
    } else {
        ptr_1->pos_lst = std::move(ptr_2->pos_lst);
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

Bytes BpTree::find_r(const Value &key, nodePtrType ptr) const{
    bool is_leaf = ptr->is_leaf;
    if (ptr == nullptr){
        throw_error("Error: can't fount key");
    }
    for (auto &x: ptr->pos_lst) {
        if (key == x.first && is_leaf){
            Record record(table_property);
            PosList pos_lst;
            pos_lst.push_back(x.second);
            return record.read_record(pos_lst)[0];
        } else if (!is_leaf && key <= x.first){
            return find_r(key, read(x.second));
        } else if (key < x.first && is_leaf) {
            break;
        }
    }
    throw_error("Error: can't fount key");
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
                auto left_ptr = insert_r(key, data, node);
                if (left_ptr){
                    write(left_ptr);
                    ptr->pos_lst.insert(iter, std::make_pair(left_ptr->last_key(), left_ptr->file_pos));
                }
            }
            is_for_end = false;
            break;
        }
    }
    if (is_for_end) {
        auto lst_end_ptr = std::prev(ptr->pos_lst.end());
        if (is_leaf){
            Record record(table_property);
            Pos record_pos = record.insert_record(data);
            ptr->pos_lst.insert(ptr->pos_lst.end(), std::make_pair(key, record_pos));
        } else {
            auto node = read(lst_end_ptr->second);
            auto left_ptr = insert_r(key, data, node);
            if (left_ptr){
                write(left_ptr);
                ptr->pos_lst.insert(lst_end_ptr, std::make_pair(left_ptr->last_key(), left_ptr->file_pos));
            }
            lst_end_ptr->first = key;
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

