#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <utility>

#include "bpTree.h"
#include "table.h"
#include "util.h"
#include "io.h"


using std::ios;
using std::vector;
using DB::Const::BLOCK_SIZE;
using DB::Type::Pos;


std::string Table::get_file_abs_path(bool is_index)const {
    namespace bf = boost::filesystem;
    auto dir_path = bf::path(__FILE__).parent_path();
    std::string file_path = dir_path.generic_string()+"/db_file/";
    if (is_index) {
        file_path += property.table_name + "_index.sdb";
    } else {
        file_path += property.table_name + ".sdb";
    }
    return file_path;
}

DB::Type::Bytes Table::tuple_get_col_value(const DB::Type::Bytes &tuple,
                                const std::string &col_name) const {
    Pos pos = 0;
    for (auto &&item : property.col_property) {
        if (item.first == col_name) {
            auto col_type = item.second.first;
            auto col_size = item.second.second;
            if (col_type == DB::Enum::VARCHAR) {
                std::string str(col_size, '\0');
            }
        }
        pos += item.second.second;
    }
}

// SQL
DB::Type::Bytes Table::make_tuple(std::initializer_list<std::string> args) {
}

void Table::insert(std::initializer_list<std::string> args) {
}

void Table::update(const std::string &col_name,
                   const std::string &op,
                   const std::string &value) {
}

void Table::remove(const std::string &col_name,
                   const std::string &op,
                   const std::string &value) {

}
