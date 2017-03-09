#include <boost/filesystem.hpp>
#include <iostream>
#include "util.h"

namespace DB {
    namespace Type {
        // ========== Value =========
        Value Value::str_to_value(Enum::ColType col_type, const std::string &str) {
            switch (col_type) {
                case Enum::INT:
                    return make(col_type, std::stoi(str));
                case Enum::FLOAT:
                    return make(col_type, std::stof(str));
                default:
                    return make(col_type, str);
            }
        }
        // ========== ColProperty =========
        Enum::ColType ColProperty::get_col_type(const std::string &col_name)const {
            return get_tuple(col_name).col_type;
        }
        size_t ColProperty::get_type_size(const std::string &col_name)const {
            return get_tuple(col_name).type_size;
        }
        ColProperty::Tuple ColProperty::get_tuple(const std::string &col_name)const {
            for (auto &&tuple : tuple_lst) {
                if (tuple.col_name == col_name) {
                    return tuple;
                }
            }
            throw std::runtime_error(
                    std::string("Error: get_tuple can't found col name{")+col_name+"}!"
            );
        }
        void ColProperty::push_back(const std::string &col_name, Enum::ColType col_type, size_t type_size) {
            tuple_lst.push_back(Tuple(col_name, col_type, type_size));
        }

        // ========== TableProperty =========
        size_t TableProperty::get_record_size() const {
            size_t total_size = 0;
            for (auto &&item : col_property.tuple_lst) {
                total_size += item.type_size;
            }
            return total_size;
        }
    }
}