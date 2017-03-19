//
// Created by sven on 17-3-18.
//

#ifndef MAIN_CACHE_H
#define MAIN_CACHE_H

#include "util.h"
#include <memory>
#include <list>
#include <unordered_map>

// LRU
class Cache {
public:
    // type
    using Bytes = DB::Type::Bytes;
    using BytesPtr = std::shared_ptr<Bytes>;
    using CacheKey = std::string;
    using KeyLst = std::list<CacheKey>;
    using CountLst = std::list<std::pair<size_t, KeyLst>>;

    // struct
    struct CacheValue {
        BytesPtr ptr;
        CountLst::iterator count_iter;
        KeyLst::iterator key_iter;

        CacheValue():ptr(nullptr), count_iter(CountLst::iterator()), key_iter(KeyLst::iterator()){}
        CacheValue(BytesPtr ptr, CountLst::iterator count_iter, KeyLst::iterator key_iter)
                :ptr(ptr), count_iter(count_iter), key_iter(key_iter){}
    };

    Cache(size_t max_block_count):max_block_count(max_block_count){}

    BytesPtr get(const std::string &path, size_t block_num);
    void put(const std::string &path, size_t block_num, BytesPtr ptr);
    // get data
    std::list<CacheValue> get_data()const;

private:
    size_t max_block_count;
    CountLst count_lst;
    std::unordered_map<CacheKey, CacheValue> data;

private:
    CacheKey generate_key(const std::string &path, size_t block_num);
};

#endif //MAIN_CACHE_H
