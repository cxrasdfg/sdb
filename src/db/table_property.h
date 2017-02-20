//
// Created by sven on 17-2-20.
//

#ifndef METATABLE_H
#define METATABLE_H

#include <string>
#include <map>

struct TableProperty {
    std::string table_name;
    std::map<std::string, std::pair<char, size_t>> name_type;

    TableProperty()= delete;
    TableProperty(const std::string &table_name,
              const std::map<std::string, std::pair<char, size_t >> &name_type)
            :table_name(table_name), name_type(name_type){}

    size_t get_record_size()const;

};


#endif //METATABLE_H
