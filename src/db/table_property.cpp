//
// Created by sven on 17-2-20.
//

#include "table_property.h"

size_t TableProperty::get_record_size() const {
    size_t total_size = 0;
    for (auto &&item : name_type) {
        total_size += item.second.second;
    }
    return total_size;
}
