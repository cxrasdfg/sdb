//
// Created by sven on 17-2-19.
//

#ifndef UTIL_H
#define UTIL_H
#include <stdexcept>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <cppformat/format.h>


namespace DB {
    namespace Enum {
        enum ColType: char {
            // int
            INT,
            // float
            FLOAT,
            CHAR,
            VARCHAR
        };
    }

    namespace Const {
        #include <unistd.h>
        const size_t BLOCK_SIZE = size_t(getpagesize());
        const size_t SIZE_SIZE = sizeof(size_t);
        const size_t INT_SIZE = sizeof(int32_t);
        const size_t FLOAT_SIZE = sizeof(float);
        const size_t POS_SIZE = sizeof(size_t);
    }

    namespace Type {
        using Int = int32_t;
        using String = std::string;
        using Float = double;
        using Pos = size_t;
        using PosList = std::vector<Pos>;
        using Byte = char;
        using Bytes = std::vector<Byte>;
        using BytesList = std::vector<Bytes>;

        struct Value {
            // data
            DB::Enum::ColType type;
            Bytes data;

            // function
            Value()= delete;
            Value(DB::Enum::ColType type, Bytes data)
                    :type(type), data(data){}

            // value_op
            template <typename Func>
            static auto value_op(const Value &value1, const Value &value2, Func op) {
                using namespace Enum;
                using namespace Const;
                if (value1.type != value2.type) {
                    throw std::runtime_error(
                            fmt::format("Error: type {0} {1} can't call op function")
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
                    case CHAR:
                    case VARCHAR:
                        std::string v1(value1.data.begin(), value1.data.end());
                        std::string v2(value2.data.begin(), value2.data.end());
                        return op(v1, v2);
//                    default:
//                        throw std::runtime_error("Error: Value type error");
                }
            }
            template <typename Func>
            auto value_op(const Value &value, Func op) {
                using namespace Enum;
                using namespace Const;
                switch (value.type) {
                    case INT:
                        Int i;
                        std::memcpy(&i, value.data.data(), Const::INT_SIZE);
                        return op(i);
                    case FLOAT:
                        float f;
                        std::memcpy(&f, value.data.data(), Const::FLOAT_SIZE);
                        return op(f);
                    case CHAR:
                    case VARCHAR:
                        std::string v(value.data.begin(), value.data.end());
                        return op(v);
//                    default:
//                        throw std::runtime_error("Error: Value type error");
                }
            }
            template <typename Func>
            auto value_op(Func op) {
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
                    case CHAR:
                    case VARCHAR:
                        std::string v(data.begin(), data.end());
                        return op(v);
//                    default:
//                        throw std::runtime_error("Error: Value type error");
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

            //
            std::string get_string(){
                return value_op([=](auto x){ return str_ret(x);});
            }

        private:
            template <typename T> std::string str_ret(T t){
                return std::to_string(t);
            }
            std::string str_ret(std::string str) {
                return str;
            }
        };

        struct ColProperty{
            // type
            struct Tuple {
                Tuple(const std::string &col_name, Enum::ColType col_type, size_t type_size)
                        :col_name(col_name), col_type(col_type), type_size(type_size){}
                std::string col_name;
                Enum::ColType col_type;
                size_t type_size;
            };
            // data member
            std::vector<Tuple> tuple_lst;

            // function
            Enum::ColType get_col_type(const std::string &col_name)const;
            size_t get_type_size(const std::string &col_name)const;
            Tuple get_tuple(const std::string &col_name)const;
            void push_back(const std::string &col_name, Enum::ColType col_type, size_t type_size);
        };

        struct TableProperty {
            // Type
            std::string table_name;
            std::string key;
            ColProperty col_property;

            TableProperty(){}
            TableProperty(const std::string &table_name,
                          const std::string &key,
                          const ColProperty &col_property)
                    :table_name(table_name), key(key), col_property(col_property){}

            size_t get_record_size()const;
        };
    }

    namespace Function {
        template <typename T>
        DB::Type::Bytes en_bytes(const T &t){
            DB::Type::Bytes bytes = std::vector<char>(sizeof(t));
            std::memcpy(bytes.data(), &t, sizeof(t));
            return bytes;
        }

        inline void bytes_print(const DB::Type::Bytes &bytes) {
            for (auto &&item : bytes) {
                std::cout << item;
            }
            std::cout << std::endl;
        }
    }

}
#endif //UTIL_H
