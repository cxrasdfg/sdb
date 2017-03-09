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

struct BptNode {
    // type
//    using lstSecType = boost::variant<std::shared_ptr<Bytes>, std::shared_ptr<BptNode>>;
//    using lstItemType = std::pair<Value, lstSecType>;
//    using lstType = std::list<lstItemType>;
    using Value = DB::Type::Value;
    using Bytes = DB::Type::Bytes;
    using Pos = DB::Type::Pos;
    using lstPosItemType = std::pair<DB::Type::Value, DB::Type::Pos>;
    using PoslstType = std::list<lstPosItemType>;
    using nodePtrType = std::shared_ptr<BptNode>;

//    std::list<std::pair<Value, lstSecType>> lst;
    std::list<std::pair<Value, Pos>> pos_lst;
//    std::shared_ptr<BptNode> end_ptr;
    bool is_leaf = true;
    bool is_new_node = true;
    Pos end_pos = 0;
    Pos file_pos = 0;

    // === 节点操作 ===
    // 获取节点最后的key
    Value last_key()const;
    
    // lstSecType 
//    static std::shared_ptr<Bytes> &get_data_ptr(lstSecType &sec){
//        return boost::get<std::shared_ptr<Bytes>>(sec);
//    }
//    static std::shared_ptr<BptNode> &get_node_ptr(lstSecType &sec){
//        return boost::get<std::shared_ptr<BptNode>>(sec);
//    }
};

class BpTree{
public:
    // === type ===
    using Value = DB::Type::Value;
    using Bytes = DB::Type::Bytes;
    using Pos = DB::Type::Pos;
    using nodePtrType = std::shared_ptr<BptNode>;
    using nodePosLstType = typename BptNode::PoslstType;
//    using nodeLstItemType = typename BptNode<Value, Bytes>::lstItemType;
//    using nodeLstType = typename BptNode<Value, Bytes>::lstType;

    BpTree()= delete;
    BpTree(const DB::Type::TableProperty &table_property);
    // 禁止树的复制，防止文件读写不一致
    BpTree(const BpTree &bpt)= delete;
    BpTree(BpTree &&bpt)= delete;
    const BpTree &operator=(const BpTree &bpt)= delete;
    BpTree &operator=(BpTree &&bpt)= delete;
    ~BpTree();
    void clear();
    void write_info_block();

    void insert(const Value &key, const Bytes &data);
    void remove(const Value &key);
    nodePtrType read(DB::Type::Pos pos) const;
    void write(nodePtrType ptr);
    Bytes find(const Value &key)const{return find_r(key, read(root_pos));}
    void print()const;

private:
//    nodePtrType root;
    Pos root_pos;
    DB::Type::PosList free_pos_list;
    Pos free_end_pos;
    size_t node_key_count;
    DB::Type::TableProperty table_property;

    void initialize();

    bool is_node_less(nodePtrType ptr)const;
    // 分裂节点
    nodePtrType node_split(nodePtrType &ptr);
    // 合并节点
    bool node_merge(nodePtrType &ptr_1, nodePtrType &ptr_2);

    // 递归插入
    nodePtrType insert_r(const Value &key, const Bytes &data, nodePtrType ptr);
    // 递归删除
    bool remove_r(const Value &key, nodePtrType &ptr);
    Bytes find_r(const Value &key, nodePtrType ptr) const;
    // === 异常处理 ===
    void throw_error(const std::string &str)const{
        throw std::runtime_error(str);
    }
};

#endif /* BPTREE_H */
