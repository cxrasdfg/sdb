#include <algorithm>

#include "cache.h"
#include "util.h"
#include "io.h"

using namespace SDB;

// ========== static ==========
Cache::CountLst Cache::count_lst;
std::unordered_map<Cache::CacheKey, Cache::CacheValue> Cache::data;

// ========== public function ==========
Cache::Bytes Cache::get(const std::string &path, size_t block_num) {
    CacheKey key = encode_key(path, block_num);
    auto it = data.find(key);
    if (it == data.end()) {
        return Bytes();
    }
//    std::cout << "begin: ";
//    for (auto &&x : count_lst) {
//        std::cout << "count: " << x.first;
//        std::cout << " set: ";
//        for (auto &&y : x.second) {
//            std::cout << y << " ";
//        }
//    }
//    std::cout << std::endl;
//    std::cout << data.size() << std::endl;
//    std::cout << std::endl;
    auto count_iter = it->second.count_iter;
    auto key_iter = it->second.key_iter;
    auto lst_it_next = std::next(count_iter);
    if (lst_it_next != count_lst.end() && count_iter->first == lst_it_next->first-1) {
        lst_it_next->second.push_front(key);
        it->second.count_iter = std::next(count_iter);
        it->second.key_iter = lst_it_next->second.begin();
        count_iter->second.erase(key_iter);
        if (count_iter->second.empty()) {
            count_lst.erase(count_iter);
        }
    } else {
        KeyLst key_lst{key};
        count_lst.insert(lst_it_next, make_pair(count_iter->first+1, key_lst));
        it->second.count_iter = std::next(count_iter);
        it->second.key_iter = std::next(count_iter)->second.begin();
        count_iter->second.erase(key_iter);
        if (count_iter->second.empty()) {
            count_lst.erase(count_iter);
        }
    }
//    cout << "end: ";
//    for (auto &&x : lst) {
//        cout << "count: " << x.first;
//        cout << " set: ";
//        for (auto &&y : x.second) {
//            cout << y << " ";
//        }
//    }
//    cout << endl;
    return *it->second.ptr;
}

void Cache::put(const std::string &path, size_t block_num, const Bytes &bytes) {
    BytesPtr ptr = std::make_shared<Bytes>(bytes);
    CacheKey key = encode_key(path, block_num);
    auto it = data.find(key);
    if (it != data.end()) {
        get(path, block_num);
        it->second.ptr = ptr;
        return;
    } else if (data.size() >= Const::CACHE_COUNT) {
        auto fst = count_lst.begin();
        CacheKey erase_key = fst->second.back();
        // write back file
        auto key_pair = decode_key(erase_key);
        write_back(key_pair.first, key_pair.second, *data.at(erase_key).ptr);
        // delete erase_key
        data.erase(erase_key);
        fst->second.pop_back();
        if (fst->second.empty()){
            count_lst.erase(count_lst.begin());
        }
    }
    KeyLst::iterator int_it;
    if (!count_lst.empty() && count_lst.begin()->first == 0) {
        count_lst.begin()->second.push_front(key);
    } else {
        KeyLst key_lst{key};
        count_lst.push_front(make_pair(0, key_lst));
    }
    data[key] = CacheValue(ptr, count_lst.begin(), count_lst.begin()->second.begin());
}

Cache::Bytes Cache::read_block(const std::string &path, size_t block_num) {
    // read cache
    Bytes cache_bytes = get(path, block_num);
    if (!cache_bytes.empty()) {
        return cache_bytes;
    }
    // else read file
    IO io(path);
    cache_bytes = io.read_block(block_num);
    // put data to cache
    put(path, block_num, cache_bytes);
    return cache_bytes;
}

void Cache::write_block(const std::string &path, size_t block_num, const Bytes &bytes) {
    put(path, block_num, bytes);
}

// ========== private function ==========
Cache::CacheKey Cache::encode_key(const std::string &path, size_t block_num) {
    return path+"+"+std::to_string(block_num);
}

Cache::KeyPair Cache::decode_key(const std::string &key) {
    size_t iter = key.find('+');
    std::string filename(key, 0, iter);
    size_t block_num = std::stoul(std::string(key, iter+1, key.size()));
    return std::make_pair(filename, block_num);
}

void Cache::write_back(const std::string &path, size_t block_num, const Bytes &bytes) {
    IO io(path);
    io.write_block(bytes, block_num);
}
