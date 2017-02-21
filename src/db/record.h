#ifndef MAIN_RECORD_H
#define MAIN_RECORD_H

#include <vector>
#include <string>

#include "table_property.h"
#include "util.h"

class RecordList {
public:
    RecordList()= delete;
    RecordList(const TableProperty &property):property(property){}

public:
    DB::Type::BytesList record_tuple_lst;

private:
    TableProperty property;
};

#endif //MAIN_RECORD_H
