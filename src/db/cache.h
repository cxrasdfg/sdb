//
// Created by sven on 17-3-18.
//

#ifndef MAIN_CACHE_H
#define MAIN_CACHE_H

#include "util.h"
#include <memory>
#include <list>

class Cache {
    // type
    using Bytes = DB::Type::Bytes;
    using BytesPtr = std::shared_ptr<Bytes>;

    // struct
    struct CacheNode {
        std::string path;
        size_t block_num;
        BytesPtr ptr;

        CacheNode(const std::string &path, size_t block_num, BytesPtr ptr)
                :path(path), block_num(block_num), ptr(ptr){}
    };

    BytesPtr find(const std::string &path, size_t block_num) const;
    void add_node(const std::string &path, size_t block_num, BytesPtr ptr);
    // get data
    std::list<CacheNode> get_data()const;

private:
    size_t max_block_count;
    std::list<CacheNode> data;
};


#endif //MAIN_CACHE_H
