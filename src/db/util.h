//
// Created by sven on 17-2-19.
//

#ifndef UTIL_H
#define UTIL_H

#include <stdexcept>

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
        const size_t BLOCK_SIZE = 4096;
    }

    namespace Function {
        size_t get_col_type_size(char col_type){
            switch (col_type) {
                case DB::Enum::CHAR :
                    return 1;
                case DB::Enum::INT :
                    return 4;
                case DB::Enum::FLOAT :
                    return 8;
                default:
                    throw std::runtime_error("Error: col_type");
            }
        }
    }

    namespace Type {
        using Pos = size_t;
    }

}


#endif //UTIL_H
