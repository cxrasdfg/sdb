#include <boost/filesystem.hpp>
#include <iostream>
#include "util.h"

namespace DB {
    namespace Type {
        // ========== TableProperty =========
        size_t TableProperty::get_record_size() const {
            size_t total_size = 0;
            for (auto &&item : col_property) {
                total_size += item.second.second;
            }
            return total_size;
        }
    }
}