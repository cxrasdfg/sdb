#ifndef BPTREE_H
#define BPTREE_H

#include <memory>
#include <list>
#include <stdexcept>
#include <iostream>
#include <string>
#include <boost/variant.hpp>

#include "util.h"
#include "io.h"
#include "record.h"

using DB::Type::Bytes;
using DB::Type::Pos;

template <typename KeyType, typename DataType>
struct BptNode {
    // type
    using lstSecType = boost::variant<std::shared_ptr<DataType>, std::shared_ptr<BptNode>>;
    using lstItemType = std::pair<KeyType, lstSecType>;
    using lstPosItemType = std::pair<KeyType, Pos>;
    using lstType = std::list<lstItemType>;
    using PoslstType = std::list<lstPosItemType>;
    using nodePtrType = std::shared_ptr<BptNode>;

    std::list<std::pair<KeyType, lstSecType>> lst;
    std::list<std::pair<KeyType, Pos>> pos_lst;
    std::shared_ptr<BptNode> end_ptr;
    Pos end_pos;
    Pos file_pos;
    bool is_leaf = false;

    // === 节点操作 ===
    // 分裂节点
    std::shared_ptr<BptNode> split();
    // 获取节点最后的key
    KeyType last_key()const;
    
    // lstSecType 
    static std::shared_ptr<DataType> &get_data_ptr(lstSecType &sec){
        return boost::get<std::shared_ptr<DataType>>(sec);
    }
    static std::shared_ptr<BptNode> &get_node_ptr(lstSecType &sec){
        return boost::get<std::shared_ptr<BptNode>>(sec);
    }
};

template <typename KeyType, typename DataType>
class BpTree{
public:
    // === type ===
    using nodeType = BptNode<KeyType, DataType>;
    using nodePtrType = std::shared_ptr<nodeType>;
    using nodeLstItemType = typename BptNode<KeyType, DataType>::lstItemType;
    using nodeLstType = typename BptNode<KeyType, DataType>::lstType;

    BpTree()= delete;
    BpTree(const DB::Type::TableProperty &table_property);
    BpTree(const BpTree &bpt);
    BpTree(BpTree &&bpt);
    const BpTree &operator=(const BpTree &bpt);
    BpTree &operator=(BpTree &&bpt);
    ~BpTree();
    void clear(){
        root = nullptr;
    }

    void set_node_key_count();

    void insert(const KeyType &key, const DataType &data);
    void remove(const KeyType &key);
    nodePtrType read(DB::Type::Pos pos);
    void write(const nodePtrType &ptr);
    DataType find(const KeyType &key)const{return find_r(key, root);}
    nodePtrType clone()const{ return clone_r(root);}
    void print()const;

private:
    nodePtrType root;
    size_t node_key_count;
    DB::Type::TableProperty table_property;

    bool is_node_less(nodePtrType ptr)const;
    // 分裂节点
    nodePtrType node_split(nodePtrType &ptr);
    // 合并节点
    bool node_merge(nodePtrType &ptr_1, nodePtrType &ptr_2);
    // 递归插入
    nodePtrType insert_r(const KeyType &key, const DataType &data, nodePtrType ptr);
    // 递归删除
    bool remove_r(const KeyType &key, nodePtrType &ptr);
    DataType find_r(const KeyType &key, nodePtrType ptr)const;
    nodePtrType clone_r(nodePtrType ptr)const;
    // === 异常处理 ===
    void throw_error(const std::string &str)const{
        throw std::runtime_error(str);
    }
};

// --------------- Function ---------------
// ========== BptNode Function =========
template <typename KeyType, typename DataType>
std::shared_ptr<BptNode<KeyType, DataType>> BptNode<KeyType, DataType>::split(){
    auto left_ptr = std::make_shared<decltype(lst)>();
    left_ptr->splice(left_ptr->begin(), lst, lst.begin(), std::next(lst.begin(), lst.size()/2));
    auto left_node_ptr = std::make_shared<BptNode>();
    left_node_ptr->lst = *left_ptr;
    if (!is_leaf()){
        left_node_ptr->end_ptr = get_node_ptr(lst.front().second);
        lst.pop_front();
    }
    return left_node_ptr;
}

template <typename KeyType, typename DataType>
KeyType BptNode<KeyType, DataType>::last_key()const{
    return (--lst.end())->first;
}

// ========== BpTree Function =========
// ---------- BpTree Public Function ---------
template <typename KeyType, typename DataType>
BpTree<KeyType, DataType>::BpTree(const DB::Type::TableProperty &table_property):table_property(table_property){
    set_node_key_count();
}

template <typename KeyType, typename DataType>
BpTree<KeyType, DataType>::BpTree(const BpTree &bpt):table_property(bpt.table_property){
    *this = bpt;
}

template <typename KeyType, typename DataType>
BpTree<KeyType, DataType>::BpTree(BpTree &&bpt):table_property(bpt.table_property){
    *this = bpt;
}

template <typename KeyType, typename DataType>
const BpTree<KeyType, DataType> &BpTree<KeyType, DataType>::operator=(const BpTree &bpt){
    if (this != &bpt){
        clear();
        node_key_count = bpt.node_key_count;
        table_property = bpt.table_property;
        root = bpt.clone();
    }
    return *this;
}

template <typename KeyType, typename DataType>
BpTree<KeyType, DataType> &BpTree<KeyType, DataType>::operator=(BpTree &&bpt) {
    if (this != &bpt) {
        clear();
        node_key_count = bpt.node_key_count;
        table_property = bpt.table_property;
        root = bpt.root;
        bpt.root = nullptr;
    }
    return *this;
}

template <typename KeyType, typename DataType>
BpTree<KeyType, DataType>::~BpTree(){
    clear();
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::set_node_key_count() {
    KeyType key_size = table_property.col_property[table_property.key].second;
    size_t pos_size = sizeof(DB::Type::Pos);
    node_key_count = (DB::Const::BLOCK_SIZE-pos_size)/(key_size+pos_size);
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::insert(const KeyType &key, const DataType &data){
    if (root == nullptr){
        auto new_ptr = read(0);
        if (new_ptr->pos_lst.empty()) {
            root = std::make_shared<nodeType>();
            root->lst.push_back(std::make_pair(key, std::make_shared<DataType>(data)));
            write(root);
            return;
        }
        root = new_ptr;
    }
    auto left_ptr = insert_r(key, data, root);
    if (left_ptr != nullptr){
        auto new_root = std::make_shared<nodeType>();
        new_root->lst.push_back(std::make_pair(left_ptr->last_key(), left_ptr));
        new_root->lst.push_back(std::make_pair(root->last_key(), root));
        root = new_root;
        write(root);
    }
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::remove(const KeyType &key){
    if (root == nullptr){
        throw_error("Error: B+ Tree is empty");
    }
    remove_r(key, root);
    if (root->lst.size() == 1){
        root = root->get_node_ptr(root->lst.begin()->second);
    }
}

template <typename KeyType, typename DataType>
typename BpTree<KeyType, DataType>::nodePtrType
BpTree<KeyType, DataType>::read(DB::Type::Pos pos) {
    using DB::Function::de_bytes;
    std::string path = table_property.get_file_abs_path(true);
    IO io(path);
    Bytes block_data = io.read_block(pos);

    nodePtrType ptr = std::make_shared<nodeType>();

    bool is_leaf = (block_data[0]!='\0');
    ptr->is_leaf = is_leaf;
    auto beg = block_data.data()+1;
    size_t key_len = table_property.col_property[table_property.key].second;
    size_t item_len = key_len + sizeof(pos);
    for (size_t i = 0; i < node_key_count; ++i) {
        Bytes item_bytes(beg+(i*item_len), beg+((i+1)*item_len));
        auto item_beg = beg+(i*item_len);
        auto item_tem = item_beg+key_len;
        auto item_end = item_tem + sizeof(pos);
        KeyType key;
        Pos child_pos;
        de_bytes(Bytes(item_beg, item_tem), key);
        de_bytes(Bytes(item_tem, item_end), child_pos);
        std::pair<KeyType, Pos> item(key, child_pos);
        ptr->pos_lst.push_back(item);
    }
    if (is_leaf) {
        de_bytes(Bytes(beg, beg+(node_key_count*item_len)), ptr->end_pos);
    }
    return ptr;
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::write(const nodePtrType &ptr) {
    using DB::Function::en_bytes;
    Bytes block_data(DB::Const::BLOCK_SIZE);
    block_data[0] = ptr->is_leaf;
    auto beg = block_data.data() + 1;
    size_t offset = 0;
    size_t key_len = table_property.col_property[table_property.key].second;
    size_t item_len = key_len + sizeof(Pos);
    for (auto &&item : ptr->lst) {
        Bytes key_bytes;
        en_bytes(key_bytes, item.first);
        std::memcpy(beg+offset, key_bytes.data(), key_len);
        std::memcpy(beg+offset+key_len, item.second, sizeof(Pos));
        offset += item_len;
    }
    if (ptr->end_pos) {
        std::memcpy(beg+offset, &ptr->end_pos, key_len);
    }

    IO io(table_property.get_file_abs_path(true));
    io.write_block(block_data, ptr->file_pos);
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::print()const{
    std::list<nodePtrType> deq;
    deq.push_back(root);
    size_t sub_count = 1;
    while (!deq.empty()) {
        auto ptr = deq.front(); 
        deq.pop_front();
        sub_count--;
        if (ptr == nullptr){
            std::cout << "nullptr";
            continue;
        }
        bool is_leaf = ptr->is_leaf();
        std::cout << "[ ";
        for (auto iter = ptr->lst.begin(); iter != ptr->lst.end(); iter++){
            if (is_leaf){
                std::cout << iter->first << ":";
                std::cout << *(ptr->get_data_ptr(iter->second)) << " ";
            } else {
                std::cout << iter->first << " ";
                deq.push_back(ptr->get_node_ptr(iter->second));
            }
        }
        std::cout << "]";
        if (sub_count == 0){
            sub_count = deq.size();
            std::cout << std::endl;
        }
    }
}

// ---------- BpTree Private Function ---------
template <typename KeyType, typename DataType>
typename BpTree<KeyType, DataType>::nodePtrType
BpTree<KeyType, DataType>::node_split(nodePtrType &ptr) {
    auto left_lst_ptr = std::make_shared<nodeLstType>();
    left_lst_ptr->splice(left_lst_ptr->begin(), ptr->lst,
                         ptr->lst.begin(), std::next(ptr->lst.begin(), ptr->lst.size()/2));
    auto left_node_ptr = std::make_shared<nodeType>();
    left_node_ptr->lst = *left_lst_ptr;
    if (ptr->is_leaf()){
        left_node_ptr->end_ptr = ptr;
    }
    return left_node_ptr;
}

template <typename KeyType, typename DataType>
bool BpTree<KeyType, DataType>::node_merge(nodePtrType &ptr_1, nodePtrType &ptr_2){
    ptr_2->lst.splice(ptr_2->lst.begin(), ptr_1->lst);
    if (ptr_2->lst.size() > node_key_count){
        ptr_1 = ptr_2->split();
        return false;
    } else {
        ptr_1 = ptr_2;
        ptr_2 = nullptr;
        return true;
    }
}

template <typename KeyType, typename DataType>
bool BpTree<KeyType, DataType>::is_node_less(nodePtrType ptr)const{
    return (node_key_count/2) > ptr->lst.size();
}

template <typename KeyType, typename DataType>
DataType BpTree<KeyType, DataType>::find_r(const KeyType &key, nodePtrType ptr)const{
    if (ptr == nullptr){
        throw_error("Error: can't fount key");
    }
    bool is_leaf = ptr->is_leaf();
    for (auto &x: ptr->lst) {
        if (key == x.first){
            if (is_leaf)
                return *(ptr->get_data_ptr(x.second));
            else
                return find_r(key, ptr->get_node_ptr(x.second));
        } else if (key < x.first){
            return find_r(key, ptr->get_node_ptr(x.second));
        }
    }
    throw_error("Error: can't fount key");
}

template <typename KeyType, typename DataType>
typename BpTree<KeyType, DataType>::nodePtrType
BpTree<KeyType, DataType>::clone_r(nodePtrType ptr) const {
    nodePtrType clone_ptr = std::make_shared<nodeType>(*ptr);
    for (auto item = clone_ptr->lst.begin(); item != clone_ptr->lst.end(); item++) {
        if (ptr->is_leaf()) {
            item->second = std::make_shared<DataType>(*ptr->get_data_ptr(item->second));
        } else {
            item->second = clone_r(ptr->get_node_ptr(item->second));
            nodePtrType item_sec_ptr = ptr->get_node_ptr(item->second);
            if (item_sec_ptr->is_leaf() && item != clone_ptr->lst.begin()){
                ptr->get_node_ptr(std::prev(item)->second)->end_ptr = item_sec_ptr->end_ptr;
            }
        }
    }
    return clone_ptr;
}

template <typename KeyType, typename DataType>
typename BpTree<KeyType, DataType>::nodePtrType
BpTree<KeyType, DataType>::insert_r(const KeyType &key, const DataType &data, nodePtrType ptr){
    bool is_leaf = ptr->is_leaf();
    bool is_for_end = true;
    for (auto iter=ptr->lst.begin(); iter!=ptr->lst.end(); iter++){
        if (iter->first == key){
            throw_error("Error: key already existed!");
        } else if (key < iter->first) {
            if (is_leaf){
                ptr->lst.insert(iter, std::make_pair(key, std::make_shared<DataType>(data)));
            } else {
                auto left_ptr = insert_r(key, data, ptr->get_node_ptr(iter->second));
                if (left_ptr){
                    ptr->lst.insert(iter, std::make_pair(left_ptr->last_key(), left_ptr));
                }
            }
            is_for_end = false;
            break;
        }
    }
    if (is_for_end) {
        auto lst_end_ptr = std::prev(ptr->lst.end());
        if (is_leaf){
            ptr->lst.insert(ptr->lst.end(), std::make_pair(key, std::make_shared<DataType>(data)));
        } else {
            auto left_ptr = insert_r(key, data, ptr->get_node_ptr(lst_end_ptr->second));
            if (left_ptr){
                ptr->lst.insert(lst_end_ptr, std::make_pair(left_ptr->last_key(), left_ptr));
            }
            lst_end_ptr->first = key;
        }
    }
    return (ptr->lst.size() > node_key_count)? node_split(ptr): nullptr;
}

template <typename KeyType, typename DataType>
bool BpTree<KeyType, DataType>::remove_r(const KeyType &key, nodePtrType &ptr){
    bool is_leaf = ptr->is_leaf();
    bool is_for_end = true;
    for (auto iter = ptr->lst.begin(); iter != ptr->lst.end(); iter++){
        if (is_leaf) {
            if (key == iter->first) {
                ptr->lst.erase(iter);
                is_for_end = false;
                break;
            } else if (key < iter->first) {
                throw_error("Error: can't find Key");
            }
        } else if (key <= iter->first) {
            nodePtrType &iter_sec_ptr = ptr->get_node_ptr(iter->second);
            bool is_less = remove_r(key, iter_sec_ptr);
            iter->first = iter_sec_ptr->last_key();
            if (is_less){
                bool is_one;
                if (std::next(iter) == ptr->lst.end()){
                    auto iter_prev = std::prev(iter);
                    is_one = node_merge(ptr->get_node_ptr(iter_prev->second), iter_sec_ptr);
                    if (is_one) ptr->lst.erase(iter);
                    iter_prev->first = ptr->get_node_ptr(iter_prev->second)->last_key();
                } else {
                    auto iter_next = std::next(iter);
                    is_one = node_merge(iter_sec_ptr, ptr->get_node_ptr(iter_next->second));
                    if (is_one){
                        ptr->lst.erase(iter_next);
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
    return is_node_less(ptr);
}

#endif /* BPTREE_H */
