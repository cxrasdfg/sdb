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
        // ========== TupleProperty =========
        Enum::ColType TupleProperty::get_col_type(const std::string &col_name)const {
            return get_col_property(col_name).col_type;
        }
        size_t TupleProperty::get_type_size(const std::string &col_name)const {
            return get_col_property(col_name).type_size;
        }
        TupleProperty::ColProperty TupleProperty::get_col_property(const std::string &col_name)const {
            for (auto &&tuple : property_lst) {
                if (tuple.col_name == col_name) {
                    return tuple;
                }
            }
            throw std::runtime_error(
                    std::string("Error: get_col_property can't found col name{")+col_name+"}!"
            );
        }
        void TupleProperty::push_back(const std::string &col_name, Enum::ColType col_type, size_t type_size) {
            property_lst.push_back(ColProperty(col_name, col_type, type_size));
        }

        // ========== TableProperty =========
        size_t TableProperty::get_record_size() const {
            size_t total_size = 0;
            for (auto &&item : tuple_property.property_lst) {
                total_size += item.type_size;
            }
            return total_size;
        }
    }
}