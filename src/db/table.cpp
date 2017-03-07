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

// SQL
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

void Table::new_table() {
}
