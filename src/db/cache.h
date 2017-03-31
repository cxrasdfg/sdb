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
    using Bytes = SDB::Type::Bytes;
    using BytesPtr = std::shared_ptr<Bytes>;
    using CacheKey = std::string;
    using KeyLst = std::list<CacheKey>;
    using CountLst = std::list<std::pair<size_t, KeyLst>>;
    using KeyPair = std::pair<std::string, size_t>;

    // struct
    struct CacheValue {
        BytesPtr ptr;
        CountLst::iterator count_iter;
        KeyLst::iterator key_iter;

        CacheValue():ptr(nullptr), count_iter(CountLst::iterator()), key_iter(KeyLst::iterator()){}
        CacheValue(BytesPtr ptr, CountLst::iterator count_iter, KeyLst::iterator key_iter)
                :ptr(ptr), count_iter(count_iter), key_iter(key_iter){}
    };

    static Cache &make(){
        static Cache cache;
        return cache;
    }

    // get and put
    Bytes get(const std::string &path, size_t block_num);
    void put(const std::string &path, size_t block_num, const Bytes &bytes);
    // block io
    Bytes read_block(const std::string &path, size_t block_num);
    void write_block(const std::string &path, size_t block_num, const Bytes &bytes);

private:
    Cache(){}
    Cache(const Cache &)= delete;
    Cache(Cache &&)= delete;
    Cache &operator=(const Cache &)= delete;
    Cache &operator=(Cache &&)= delete;
    ~Cache(){
        sync();
    }

    // count read/write block frequency
    CountLst count_lst;
    // block cache data
    std::unordered_map<CacheKey, CacheValue> data;

private:
    CacheKey encode_key(const std::string &path, size_t block_num);
    KeyPair decode_key(const std::string &key);
    void write_back(const std::string &path, size_t block_num, const Bytes &bytes);
    void sync();
};

#endif //CACHE_H
