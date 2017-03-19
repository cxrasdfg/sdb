#include <algorithm>
#include <boost/filesystem.hpp>

#include "cache.h"

using namespace DB;

// ========== public function ==========
Cache::BytesPtr Cache::get(const std::string &path, size_t block_num) {
    CacheKey key = generate_key(path, block_num);
    auto it = data.find(key);
    if (it == data.end()) {
        return nullptr;
    }
//    cout << "begin: ";
//    for (auto &&x : lst) {
//        cout << "count: " << x.first;
//        cout << " set: ";
//        for (auto &&y : x.second) {
//            cout << y << " ";
//        }
//    }
//    cout << endl;
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
    return it->second.ptr;
}

void Cache::put(const std::string &path, size_t block_num, BytesPtr ptr) {
    if (max_block_count == 0) {
        return;
    }
    CacheKey key = generate_key(path, block_num);
    auto it = data.find(key);
    if (it != data.end()) {
        get(path, block_num);
        it->second.ptr = ptr;
        return;
    } else if (data.size() >= max_block_count) {
        auto fst = count_lst.begin();
        CacheKey erase_key = fst->second.back();
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

// ========== private function ==========
Cache::CacheKey Cache::generate_key(const std::string &path, size_t block_num) {
    namespace bf = boost::filesystem;
    auto filename = bf::path(path).filename().generic_string();
    return filename+std::to_string(block_num);
}
