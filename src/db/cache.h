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

    // get and put
    static Bytes get(const std::string &path, size_t block_num);
    static void put(const std::string &path, size_t block_num, const Bytes &bytes);
    // block io
    static Bytes read_block(const std::string &path, size_t block_num);
    static void write_block(const std::string &path, size_t block_num, const Bytes &bytes);

private:
    static CountLst count_lst;
    static std::unordered_map<CacheKey, CacheValue> data;

private:
    static CacheKey encode_key(const std::string &path, size_t block_num);
    static KeyPair decode_key(const std::string &key);
    static void write_back(const std::string &path, size_t block_num, const Bytes &bytes);
};

#endif //MAIN_CACHE_H
