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

        // ========== Tuple =========
        Value Tuple::get_col_value(const TupleProperty &tuple_property, const std::string &col_name)const {
            auto lst = tuple_property.property_lst;
            for (size_t j = 0; j < lst.size(); ++j) {
                if (lst[j].col_name == col_name) {
                    return value_lst[j];
                }
            }
            throw std::runtime_error(std::string("Error: can't found col:{")+col_name+"}");
        }

        Value& Tuple::get_col_value_ref(const TupleProperty &tuple_property, const std::string &col_name) {
            auto lst = tuple_property.property_lst;
            for (size_t j = 0; j < lst.size(); ++j) {
                if (lst[j].col_name == col_name) {
                    return value_lst[j];
                }
            }
            throw std::runtime_error(std::string("Error: can't found col:{")+col_name+"}");
        }

        void Tuple::set_col_value(const TupleProperty &property, const std::string &col_name, const Value &value) {
            get_col_value_ref(property, col_name) = value;
        }

        void Tuple::set_col_value(const TupleProperty &property, const std::string &col_name, VVFunc op) {
            auto &value = get_col_value_ref(property, col_name);
            value = op(value);
        }

        void Tuple::set_col_value(const TupleProperty &property, const std::string &col_name,
                                  BVFunc predicate, VVFunc op) {
            Value &value = get_col_value_ref(property, col_name);
            if (predicate(value)) {
                value = op(value);
            }
        }

        void TupleLst::print() const {
            for (auto &&lst: tuple_property.property_lst) {
                std::cout << lst.col_name << "\t|";
            }
            std::cout << std::endl;
            for (auto &&item : tuple_lst) {
                for (auto &&value : item.value_lst) {
                     std::cout << value.get_string() << "\t|";
                }
                std::cout << std::endl;
            }
        }
    }

    namespace Function {
        size_t get_type_len(Enum::ColType col_type) {
            using namespace Enum;
            using namespace Const;
            switch (col_type) {
                case INT:
                    return INT_SIZE;
                case FLOAT:
                    return FLOAT_SIZE;
                default:
                    throw std::runtime_error("Error: [get_type_len] type must be non-variable type");
            }
        }

        bool is_var_type(Enum::ColType type) {
            using namespace Enum;
            return type != VARCHAR;
        }

        Type::BVFunc get_bvfunc(Enum::BVFunc func, Type::Value value) {
            using Type::Value;
            switch (func) {
                case Enum::EQ:
                    return [value](Value v){ return v == value;};
                case Enum::LESS:
                    return [value](Value v){ return v < value;};
                case Enum::GREATER:
                    return [value](Value v){ return !(v <= value);};
            }
        }
        void tuple_lst_map(Type::TupleLst &tuple_lst,
                           const std::string &col_name,
                           Type::VVFunc) {
            for (auto &&tuple : tuple_lst.tuple_lst) {
            }
        }
    }
}