#include "cache.h"
#include <algorithm>

using namespace DB;

BytesPtr Cache::find(const std::string &path, size_t block_num) const {
    auto it = std::find_if(data.begin(), data.end(), [path, block_num](CacheNode x){
        return x.path == path && x.block_num == block_num;
    });
    if (it == data.end()) {
        return nullptr;
    }
    return it->ptr;
}

void Cache::add_node(const std::string &path, size_t block_num, BytesPtr ptr) {
    CacheNode node(path, block_num, ptr);
    if (data.size() >= max_block_count) {
        data.pop_back();
    }
    data.push_back(node);
}
