#include <boost/filesystem.hpp>
#include "util.h"

namespace DB {
    namespace Function {
        std::string get_db_file_dir_path(){
            namespace bf = boost::filesystem;
            auto dir_path = bf::path(__FILE__).parent_path();
            std::string file_path = dir_path.generic_string()+"/db_file";
            return file_path;
        }
    }

    namespace Type {
        // ========== TableProperty =========
        size_t TableProperty::get_record_size() const {
            size_t total_size = 0;
            for (auto &&item : col_property) {
                total_size += item.second.second;
            }
            return total_size;
        }

        std::string TableProperty::get_file_abs_path(char file_suffix) const {
            std::string file_path = DB::Function::get_db_file_dir_path();
            file_path += '/' + table_name;
            if (file_suffix == DB::Enum::INDEX_SUFFIX) {
                file_path += "_index.sdb";
            } else if (file_suffix == DB::Enum::RECORD_SUFFIX) {
                file_path += "_record.sdb";
            } else if (file_suffix == DB::Enum::POS_SUFFIX) {
                file_path += "_pos.sdb";
            } else {
                throw std::runtime_error(
                        std::string("Error: can't found this file suffix: ")+file_suffix
                );
            }
            return file_path;
        }
    }
}