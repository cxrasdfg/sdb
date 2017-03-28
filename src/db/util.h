//
// Created by sven on 17-2-19.
//

#ifndef UTIL_H
#define UTIL_H

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <boost/spirit/home/support/container.hpp>

namespace SDB {
    namespace Enum {
        enum ColType: char {
            // int
            INT,
            // float
            FLOAT,
            VARCHAR
        };

        enum BVFunc: char {
            EQ,
            LESS,
            GREATER,
        };
    }

    namespace Const {
        #include <unistd.h>
        const size_t BLOCK_SIZE = size_t(getpagesize());
        const size_t SIZE_SIZE = sizeof(size_t);
        const size_t INT_SIZE = sizeof(int32_t);
        const size_t FLOAT_SIZE = sizeof(float);
        const size_t POS_SIZE = sizeof(size_t);

        const size_t CACHE_COUNT = 100;
    }

    namespace Type {
        using Int = int32_t;
        using String = std::string;
        using Float = float;
        using Pos = size_t;
        using PosList = std::vector<Pos>;
        using Byte = char;
        using Bytes = std::vector<Byte>;
        using BytesList = std::vector<Bytes>;

        struct Value {
            // data
            SDB::Enum::ColType type;
            Bytes data;

            // function
            Value()= delete;
            Value(SDB::Enum::ColType type, Bytes data)
                    :type(type), data(data){}

            // value_op
            template <typename Func>
            static auto value_op(const Value &value1, const Value &value2, Func op) {
                using namespace Enum;
                using namespace Const;
                if (value1.type != value2.type) {
                    throw std::runtime_error(
                            std::string("Error: type {0} {1} can't call op function")
                    );
                }
                switch (value1.type) {
                    case INT:
                        Int i1;
                        Int i2;
                        std::memcpy(&i1, value1.data.data(), Const::INT_SIZE);
                        std::memcpy(&i2, value2.data.data(), Const::INT_SIZE);
                        return op(i1, i2);
                    case FLOAT:
                        float f1;
                        float f2;
                        std::memcpy(&f1, value1.data.data(), Const::FLOAT_SIZE);
                        std::memcpy(&f2, value2.data.data(), Const::FLOAT_SIZE);
                        return op(f1, f2);
                    case VARCHAR:
                        std::string v1(value1.data.begin(), value1.data.end());
                        std::string v2(value2.data.begin(), value2.data.end());
                        return op(v1, v2);
                }
            }
            template <typename Func>
            static auto number_value_op(const Value &value1, const Value &value2, Func op){
                using namespace Enum;
                using namespace Const;
                if (value1.type != value2.type) {
                    throw std::runtime_error(
                            std::string("Error: <value_op> type {0} {1} can't call op function")
                    );
                }
                switch (value1.type) {
                    case INT:
                        Int i1;
                        Int i2;
                        std::memcpy(&i1, value1.data.data(), Const::INT_SIZE);
                        std::memcpy(&i2, value2.data.data(), Const::INT_SIZE);
                        return op(i1, i2);
                    case FLOAT:
                        float f1;
                        float f2;
                        std::memcpy(&f1, value1.data.data(), Const::FLOAT_SIZE);
                        std::memcpy(&f2, value2.data.data(), Const::FLOAT_SIZE);
                        return op(f1, f2);
                    default:
                        throw std::runtime_error("Error: type must be number type!");
                }
            }
            template <typename Func>
            auto value_op(Func op) const{
                using namespace Enum;
                using namespace Const;
                switch (type) {
                    case INT:
                        Int i;
                        std::memcpy(&i, data.data(), Const::INT_SIZE);
                        return op(i);
                    case FLOAT:
                        float f;
                        std::memcpy(&f, data.data(), Const::FLOAT_SIZE);
                        return op(f);
                    case VARCHAR:
                        std::string v(data.begin(), data.end());
                        return op(v);
                }
            }

            // make
            template <typename T>
            static Value make(Enum::ColType col_type, T data) {
                size_t type_size = sizeof(data);
                Bytes bytes(type_size);
                std::memcpy(bytes.data(), &data, type_size);
                return Value(col_type, bytes);
            }
            static Value make(Enum::ColType col_type, std::string data) {
                Bytes bytes(data.begin(), data.end());
                return Value(col_type, bytes);
            }
            static Value make(Enum::ColType col_type, Bytes data) {
                Bytes bytes(data.begin(), data.end());
                return Value(col_type, bytes);
            }
            static Value str_to_value(Enum::ColType col_type, const std::string &str);

            // operator
            friend bool operator<(const Value value1, const Value &value2) {
                return value_op(value1, value2, [](auto v1, auto v2){ return v1<v2;});
            }
            friend bool operator==(const Value &value1, const Value &value2) {
                return value_op(value1, value2, [](auto v1, auto v2){ return v1==v2;});
            }
            friend bool operator<=(const Value &value1, const Value &value2) {
                return value1 < value2 || value1 == value2;
            }
            friend Value operator+(const Value &value1, const Value &value2) {
                return number_value_op(value1, value2, [=](auto x, auto y){
                    return make(value1.type, x+y);
                });
            }
            friend Value operator-(const Value &value1, const Value &value2) {
                return number_value_op(value1, value2, [=](auto x, auto y){
                    return make(value1.type, x-y);
                });
            }
            friend Value operator*(const Value &value1, const Value &value2) {
                return number_value_op(value1, value2, [=](auto x, auto y){
                    return make(value1.type, x*y);
                });
            }
            friend Value operator/(const Value &value1, const Value &value2) {
                return number_value_op(value1, value2, [=](auto x, auto y){
                    return make(value1.type, x/y);
                });
            }

            //
            std::string get_string()const{
                return value_op([=](auto x){ return str_ret(x);});
            }
            static bool is_var_type(Enum::ColType col_type){
                return col_type == Enum::VARCHAR;
            }
            bool is_var_type()const{
                return is_var_type(type);
            }

        private:
            template <typename T> std::string str_ret(T t)const{
                return std::to_string(t);
            }
            std::string str_ret(std::string str)const {
                return str;
            }
        };

        using BVFunc = std::function<bool(Value)>;
        using VVFunc = std::function<Value(Value)>;

        // ========== Property ==========
        struct TupleProperty{
            // type
            struct ColProperty {
                ColProperty(const std::string &col_name, Enum::ColType col_type, size_t type_size)
                        :col_name(col_name), col_type(col_type), type_size(type_size){}
                std::string col_name;
                Enum::ColType col_type;
                size_t type_size;
            };
            // data member
            std::vector<ColProperty> property_lst;

            // function
            Enum::ColType get_col_type(const std::string &col_name)const;
            size_t get_type_size(const std::string &col_name)const;
            ColProperty get_col_property(const std::string &col_name)const;
            void push_back(const std::string &col_name, Enum::ColType col_type, size_t type_size);
        };

        struct TableProperty {
            // Type
            std::string db_name;
            std::string table_name;
            std::string key;
            TupleProperty tuple_property;
            // integrity
            std::unordered_map<std::string, std::string> referencing_map;
            std::unordered_map<std::string, std::string> referenced_map;
            std::unordered_set<std::string> not_null_set;

            TableProperty(){}
            TableProperty(const std::string &db_name,
                          const std::string &table_name,
                          const std::string &key,
                          const TupleProperty &col_property)
                    :db_name(db_name), table_name(table_name), key(key), tuple_property(col_property){}
        };

        struct Tuple {
            std::vector<Value> value_lst;

            Tuple()= default;
            Value get_col_value(const TupleProperty &tuple_property, const std::string &col_name)const;
            Value &get_col_value_ref(const TupleProperty &tuple_property, const std::string &col_name);
            void set_col_value(const TupleProperty &property, const std::string &col_name, const Value &value);
            void set_col_value(const TupleProperty &property, const std::string &col_name, VVFunc op);
            void set_col_value(const TupleProperty &property, const std::string &pred_col_name, BVFunc predicate,
                                           const std::string &op_col_name, VVFunc op);
        };

        struct TupleLst {
            std::vector<Tuple> tuple_lst;
            TupleProperty tuple_property;

            TupleLst()= delete;
            TupleLst(const TupleProperty &tuple_property):tuple_property(tuple_property){}

            void print()const;
        };
    }

    namespace Function {
        // === Container Traits ===
        // vector
        template <typename T>
        struct VectorTraits {
            static const bool value = false;
        };
        template <typename SubType>
        struct VectorTraits<std::vector<SubType>> {
            static const bool value = true;
        };
        // list
        template <typename SubType>
        struct ListTraits {
            static const bool value = false;
        };
        template <typename SubType>
        struct ListTraits<std::list<SubType>> {
            static const bool value = true;
        };
        // string
        template <typename SubType>
        struct StringTraits {
            static const bool value = false;
        };
        template <>
        struct StringTraits<std::string> {
            static const bool value = true;
        };
        // set
        template <typename SubType>
        struct USetTraits {
            static const bool value = false;
        };
        template <typename SubType>
        struct USetTraits<std::unordered_set<SubType>> {
            static const bool value = true;
        };
        // unordered_map
        template <typename T>
        struct UMapTraits {
            static const bool value = false;
        };
        template <typename Fst, typename Sec>
        struct UMapTraits<std::unordered_map<Fst, Sec>> {
            static const bool value = true;
        };

        // en_bytes if basic type
        using boost::spirit::traits::is_container;
        template <typename T>
        inline typename std::enable_if<!is_container<T>::value, Type::Bytes>::type
        en_bytes(T t){
            SDB::Type::Bytes bytes = std::vector<char>(sizeof(t));
            std::memcpy(bytes.data(), &t, sizeof(t));
            return bytes;
        }
        // en_bytes if 'set' type
        template <typename Ctn>
        inline typename std::enable_if<
                is_container<Ctn>::value && !std::is_same<Ctn, Type::Bytes>::value,
                Type::Bytes>::type
        en_bytes(Ctn cnt){
            Type::Bytes bytes;
            Type::Bytes size_bytes = en_bytes(cnt.size());
            bytes.insert(bytes.end(), size_bytes.begin(), size_bytes.end());
            for (auto &&x : cnt) {
                Type::Bytes x_bytes = en_bytes(x);
                bytes.insert(bytes.end(), x_bytes.begin(), x_bytes.end());
            }
            return bytes;
        }
        // en_bytes if pair type
        template <typename Fst, typename Sec>
        inline Type::Bytes en_bytes(std::pair<Fst, Sec> pair){
            Type::Bytes bytes;
            Type::Bytes fst_bytes = en_bytes(pair.first);
            Type::Bytes sec_bytes = en_bytes(pair.second);
            bytes.insert(bytes.end(), fst_bytes.begin(), fst_bytes.end());
            bytes.insert(bytes.end(), sec_bytes.begin(), sec_bytes.end());
            return bytes;
        }

        template <typename T>
        inline typename std::enable_if<UMapTraits<T>::value || USetTraits<T>::value, void>::type
        container_append(T &t, const typename T::value_type &tail) {
            t.insert(tail);
        }
        template <typename T>
        inline typename std::enable_if<VectorTraits<T>::value 
                                        || ListTraits<T>::value 
                                        || StringTraits<T>::value, void>::type
        container_append(T &t, const typename T::value_type &tail) {
            t.push_back(tail);
        }
        template <typename T>
        inline typename std::enable_if<!is_container<T>::value, void>::type
        de_bytes(T &t, const Type::Bytes &bytes, size_t &offset){
            std::memcpy(&t, bytes.data()+offset, sizeof(t));
            offset += sizeof(t);
        }

        template <typename T>
        inline typename std::enable_if<is_container<T>::value, void>::type
        de_bytes(T &t, const Type::Bytes &bytes, size_t &offset) {
            size_t len;
            std::memcpy(&len, bytes.data()+offset, Const::SIZE_SIZE);
            offset += Const::SIZE_SIZE;
            for (size_t i = 0; i < len; i++) {
                typename T::value_type value;
                de_bytes(value, bytes, offset);
                container_append(t, value);
            }
        }

        inline void bytes_print(const SDB::Type::Bytes &bytes) {
            for (auto &&item : bytes) {
                std::cout << item;
            }
            std::cout << std::endl;
        }

        template <typename T>
        inline void bytes_append(Type::Bytes &bytes, T &&tail) {
            Type::Bytes tail_bytes = en_bytes(tail);
            bytes.insert(bytes.end(), tail_bytes.begin(), tail_bytes.end());
        }

        size_t get_type_len(Enum::ColType col_type);
        bool is_var_type(Enum::ColType type);

        Type::BVFunc get_bvfunc(Enum::BVFunc func, Type::Value value);
        void tuple_lst_map(Type::TupleLst &tuple_lst, const std::string &col_name, Type::VVFunc);
    }

}
#endif //UTIL_H
