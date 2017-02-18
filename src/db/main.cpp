#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "table.h"

static std::map<std::string, char> TYPE_CHAR_MAP = {
        {"int", 0},
        {"smallint", 1},
        {"char", 2},
        {"float", 3}
};
static std::map<char, char> TYPE_SIZE_MAP = {
        {0, 4},
        {1, 2},
        {2, 1},
        {3, 4}
};

enum class En: char {
    INT = 0,
    SMALLINT = 1,
    CHAR = 2,
    FLOAT = 3
};

std::string get_type_str(const std::vector<std::string> &type_lst);
int get_record_size(const std::vector<std::string> &type_lst);
std::vector<std::string> get_test_block_data();

int main(void) {
    Table table("test", 128);
    auto data = get_test_block_data();
    for (const auto &str: data){
        std::cout << str;
    }
    std::cout << std::endl;
    table.write_block(data, 0, false);
    data = table.read_block(0, false);
    for (const auto &str: data){
        std::cout << str;
    }
    std::cout << std::endl;
    return 0;
}

std::string get_type_str(const std::vector<std::string> &type_lst){
    std::string str;
    for (auto &x: type_lst){
        str += TYPE_CHAR_MAP[x];
    }
    return str;
}

int get_record_size(const std::vector<std::string> &type_lst){
    int size = 0;
    for (auto &x: type_lst){
        size += TYPE_SIZE_MAP[TYPE_CHAR_MAP[x]];
    }
    return size;
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
