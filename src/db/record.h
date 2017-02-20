#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>

#include "table_property.h"

class RecordList {
public:
    RecordList()= delete;
    RecordList(const TableProperty &property):property(property){}

    void record_push_back(const std::string &record_tuple){
        record_tuple_lst.push_back(record_tuple);
    }

private:
    TableProperty property;
    std::vector<std::string> record_tuple_lst;
};

#endif //MAIN_RECORD_H
