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
#include "bpTree.h"

using std::cout;
using std::endl;
using DB::Type::Bytes;
using DB::Type::Pos;
using DB::Const::BLOCK_SIZE;

DB::Type::TableProperty get_table_property();
void table_init();
void bpt_test();

int main(void) {
    clock_t start = clock();
//    table_init();
    bpt_test();
    std::cout << "time:" << (double)((clock()-start))/CLOCKS_PER_SEC << std::endl;
    return 0;
}

DB::Type::TableProperty get_table_property(){
    std::map<std::string, std::pair<char, size_t >> map{
            {"col_1", std::make_pair(DB::Enum::INT, 4)},
            {"col_2", std::make_pair(DB::Enum::VARCHAR, 8)}
    };
    DB::Type::TableProperty tableProperty("test", "col_1", map);
    return tableProperty;
}

void table_init(){
    auto dir = DB::Function::get_db_file_dir_path();
    cout << dir << endl;
    // index_free.sdb
    IO io(dir+"/"+"test_index.sdb");
    Bytes bytes = io.read_block(0);
    Pos root_pos = 0;
    Pos end_pos = 0;
    size_t free_pos_count = 0;
    size_t Pos_len = sizeof(Pos);
    std::memcpy(bytes.data(), &root_pos, Pos_len);
    std::memcpy(bytes.data()+8, &free_pos_count, 8);
    std::memcpy(bytes.data()+16, &end_pos, 8);
    io.write_block(bytes, 0);
}

void bpt_test() {
    BpTree<int, DB::Type::Bytes> bpTree(get_table_property());
}
