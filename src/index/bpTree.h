#ifndef BPTREE_H
#define BPTREE_H

#include <memory>
#include <list>
#include <stdexcept>
#include <iostream>
#include <boost/variant.hpp>

template <typename KeyType, typename DataType>
struct BptNode {
    // type
    using lstSecType = boost::variant<std::shared_ptr<DataType>, std::shared_ptr<BptNode>>;

    std::list<std::pair<KeyType, lstSecType>> lst;
    std::shared_ptr<BptNode> end_ptr;

    //~BptNode(){
    //}

    // === 节点操作 ===
    // 判断是否叶节点
    bool is_leaf()const;
    // 分裂节点
    std::shared_ptr<BptNode> split();
    // 合并节点
    void merge();
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

    BpTree();
    BpTree(const BpTree &bpt){*this=bpt;}
    //BpTree(const BpTree &&bpt){*this=bpt;}
    const BpTree &operator=(const BpTree &bpt);
    //BpTree &&operator=(const BpTree &&bpt);
    ~BpTree();
    void clear(){
        clear_r(root);
        root = nullptr;
    }
    
    void insert(const KeyType &key, const DataType &data);
    void remove(const KeyType &key, const DataType &data);
    void print()const;

private:
    nodePtrType root;
    size_t block_size;
    size_t node_len;
    // 递归插入
    nodePtrType insert_r(const KeyType &key, const DataType &data, nodePtrType ptr);
    // 递归删除
    void remove_r(const KeyType &key, const DataType &data, nodePtrType &ptr);
    // 递归清理
    void clear_r(nodePtrType ptr);
    // === 异常处理 ===
    void throw_error(std::string str){
        throw std::runtime_error(str);
    }
};

// --------------- Function ---------------
// ========== BptNode Function =========
// 判断是否叶节点
template <typename KeyType, typename DataType>
bool BptNode<KeyType, DataType>::is_leaf()const{
    return !(lst.begin()->second.which());
}

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
void BptNode<KeyType, DataType>::merge(){}

template <typename KeyType, typename DataType>
KeyType BptNode<KeyType, DataType>::last_key()const{
    return (--lst.end())->first;
}

// ========== BpTree Function =========
template <typename KeyType, typename DataType>
BpTree<KeyType, DataType>::BpTree():root(nullptr), block_size(100){
    size_t key_size = sizeof(KeyType);
    size_t ptr_type_size = 64/8;
    node_len = (block_size-ptr_type_size)/(ptr_type_size+key_size);
}

template <typename KeyType, typename DataType>
BpTree<KeyType, DataType>::~BpTree(){
    clear();
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::clear_r(nodePtrType ptr){
    if (ptr == nullptr)
        return;
    bool is_leaf = ptr->is_leaf();
    for (auto iter = ptr->lst.begin(); iter != ptr->lst.end(); iter++){
        if (is_leaf){
            auto &data_ptr = BptNode<KeyType, DataType>::get_data_ptr(iter->second);
            data_ptr = nullptr;
        } else {
            auto &node_ptr = BptNode<KeyType, DataType>::get_node_ptr(iter->second);
            clear_r(node_ptr);
            node_ptr = nullptr;
        }
    }
    if (!is_leaf){
        clear_r(ptr->end_ptr);
    }
    ptr->end_ptr = nullptr;
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::insert(const KeyType &key, const DataType &data){
    if (root == nullptr){
        root = std::make_shared<nodeType>();
        root->lst.push_back(std::make_pair(key, std::make_shared<DataType>(data)));
        return;
    }
    auto left_ptr = insert_r(key, data, root);
    if (left_ptr != nullptr){
        auto new_root = std::make_shared<nodeType>();
        new_root->lst.push_back(std::make_pair(left_ptr->last_key(), left_ptr));
        new_root->end_ptr = root;
        root = new_root;
    }
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::remove(const KeyType &key, const DataType &data){

}

template <typename KeyType, typename DataType>
typename BpTree<KeyType, DataType>::nodePtrType
BpTree<KeyType, DataType>::insert_r(const KeyType &key, const DataType &data, nodePtrType ptr){
    if (ptr == nullptr){
        throw_error("Error: ptr can't be nullptr");
    }
    bool is_end = true;
    bool is_leaf = ptr->is_leaf();
    for (auto iter=ptr->lst.begin(); iter!=ptr->lst.end(); iter++){
        if (iter->first == key){
            throw_error("Error: key already existed!");
        } else if (key < iter->first) {
            if (is_leaf){
                ptr->lst.insert(iter, std::make_pair(key, std::make_shared<DataType>(data)));                
            } else {
                auto left_ptr = insert_r(key, data, BptNode<KeyType, DataType>::get_node_ptr(iter->second));
                if (left_ptr){
                    ptr->lst.insert(iter, std::make_pair(left_ptr->last_key(), left_ptr));
                }
            }
            is_end = false;
            break;
        }
    }
    if (is_end){
        if (is_leaf){
            ptr->lst.push_back(std::make_pair(key, std::make_shared<DataType>(data)));                
        } else {
            auto left_ptr = insert_r(key, data, ptr->end_ptr);
            if (left_ptr){
                ptr->lst.push_back(std::make_pair(left_ptr->last_key(), left_ptr));
            }
        }
    }
    return (ptr->lst.size() > node_len)? ptr->split(): nullptr;
}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::remove_r(const KeyType &key, 
                                         const DataType &data, 
                                         nodePtrType &ptr){}

template <typename KeyType, typename DataType>
void BpTree<KeyType, DataType>::print()const{
    std::list<nodePtrType> deq;
    deq.push_back(root);
    int sub_count = 1;
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
                std::cout << *(BptNode<KeyType, DataType>::get_data_ptr(iter->second)) << " ";
            } else {
                std::cout << iter->first << " ";
                deq.push_back(BptNode<KeyType, DataType>::get_node_ptr(iter->second));
            }
        }
        if (!is_leaf){
            deq.push_back(ptr->end_ptr);
        }
        std::cout << "]";
        if (sub_count == 0){
            sub_count = deq.size();
            std::cout << std::endl;
        }
    }
}
#endif /* BPTREE_H */
