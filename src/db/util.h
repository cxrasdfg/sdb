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


namespace DB {
    namespace Enum {
        enum : char {
            CHAR = 1,
            INT,
            FLOAT,
            VARCHAR
        };
    }

    namespace Const {
        #include <unistd.h>
        const size_t BLOCK_SIZE = size_t(getpagesize());
    }

    namespace Function {
        template <typename T>
        void de_bytes(const std::vector<char> &bytes, T &t){
            std::memcpy(&t, bytes.data(), sizeof(t));
        }
        template <>
        inline void de_bytes<std::string>(const std::vector<char> &bytes, std::string &t){
            t = std::string(bytes.begin(), bytes.end());
        }

        template <typename T>
        void en_bytes(std::vector<char> &bytes, const T &t){
            bytes = std::vector<char>(sizeof(t));
            std::memcpy(bytes.data(), &t, sizeof(t));
        }
        template <>
        inline void en_bytes<std::string>(std::vector<char> &bytes, const std::string &t){
            bytes = std::vector<char>(t.begin(), t.end());
        }
    }

    namespace Type {
        using Pos = size_t;
        using PosList = std::vector<Pos>;
        using Byte = char;
        using Bytes = std::vector<Byte>;
        using BytesList = std::vector<Bytes>;

        struct TableProperty {
            std::string table_name;
            std::string key;
            std::map<std::string, std::pair<char, size_t>> col_property;

            TableProperty()= delete;
            TableProperty(const std::string &table_name,
                          const std::string &key,
                          const std::map<std::string, std::pair<char, size_t >> &col_property)
                    :table_name(table_name), key(key),col_property(col_property){}

            size_t get_record_size()const;
        };
    }

    namespace Function {
        inline void bytes_print(const DB::Type::Bytes &bytes) {
            for (auto &&item : bytes) {
                std::cout << item;
            }
            std::cout << std::endl;
        }
    }

}


#endif //UTIL_H
