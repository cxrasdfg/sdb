#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <cstring>
#include <sys/mman.h>
#include <ctime>

//#include "table.h"


std::string get_type_str(const std::vector<std::string> &type_lst);
int get_record_size(const std::vector<std::string> &type_lst);
std::vector<std::string> get_test_block_data();

void write_test();
void string_process_test_1(){
    const int max = 1000000;
    char src[max];
    char des[max];
    for (int i = 0; i < max; ++i) {
        des[i] = src[i];
    }
}
void string_process_test_2(){
    const int max = 1000000;
    char src[max];
    char des[max];
    memcpy(des, src, max);
}

int main(void) {
    int start = clock();
    string_process_test_1();
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
