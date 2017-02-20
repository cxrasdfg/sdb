#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <cstring>
#include <sys/mman.h>
#include <ctime>

#include "table.h"
#include "util.h"
#include "table_property.h"

using std::cout;
using std::endl;


int main(void) {
    clock_t start = clock();
    std::map<std::string, std::pair<char, size_t >> map{
            {"col_1", std::make_pair(DB::Enum::INT, 4)},
            {"col_2", std::make_pair(DB::Enum::VARCHAR, 8)}
    };
    TableProperty tableProperty("test", map);
    Table table(tableProperty);
    RecordList recordList(tableProperty);
    std::vector<size_t > pos;
    for (size_t i = 0; i < 10; ++i) {
        char tuple[20];
        std::string str = "It is OK";
        std::memcpy(tuple, &i, 4);
        std::memcpy(tuple+4, str.c_str(), 8);
        std::string string(tuple, 12);
        recordList.record_tuple_lst.push_back(string);
        pos.push_back(i*12);
    }
    table.write_record(recordList, pos);
    RecordList lst(table.read_record(pos));
    for (auto &&item : lst.record_tuple_lst) {
        int num;
        char buffer[9];
        std::memcpy(&num, item.c_str(), 4);
        std::memcpy(buffer, item.c_str()+4, 8);
        buffer[8] = '\0';
        cout << "num:" << num << endl;
        cout << "str:" << buffer << endl;
    }

    std::cout << "time:" << (double)((clock()-start))/CLOCKS_PER_SEC << std::endl;
    return 0;
}

std::vector<std::string> get_test_block_data(){
    std::vector<std::string> data;
    std::string str;
    str += 7;
    str += "table_1";
    str += 3;
    str += "t_2";
    str += 5;
    str += "table";
    data.push_back(str);
    str.clear();
    str += 1;
    str += 2;
    str += 3;
    data.push_back(str);
    str.clear();
    str += "1234a33.3";
    data.push_back(str);
    str.clear();
    str += "1234a33.3";
    data.push_back(str);
    str.clear();
    str += "1234a33.3";
    data.push_back(str);
    str.clear();
    return data;
}
