#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>

#include "table_property.h"

class RecordList {
public:
    RecordList()= delete;
    RecordList(const TableProperty &property):property(property){}

public:
    std::vector<std::string> record_tuple_lst;

private:
    TableProperty property;
};

#endif //MAIN_RECORD_H
