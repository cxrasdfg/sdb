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
void io_test();
//template <typename T>
//DB::Type::Bytes make_tuple(const T &t){
//    DB::Type::Bytes tuple(sizeof(t));
//    std::memcpy(tuple.data(), &t, sizeof(t));
//    return tuple;
//}
//
//template <typename T, typename ...Args>
//DB::Type::Bytes make_tuple(const T &t, const Args& ...args){
//    DB::Type::Bytes tuple(sizeof(t));
//    std::memcpy(tuple.data(), &t, sizeof(t));
//    DB::Type::Bytes bytes = make_tuple(args...);
//    tuple.insert(tuple.end(), bytes.begin(), bytes.end());
//    return tuple;
//}

int main(void) {
    clock_t start = clock();
    table_init();
    bpt_test();
//    io_test();
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
    Pos end_pos = BLOCK_SIZE;
    size_t free_pos_count = 0;
    size_t Pos_len = sizeof(Pos);
    size_t size_len = sizeof(size_t);
    std::memcpy(bytes.data(), &root_pos, Pos_len);
    std::memcpy(bytes.data()+size_len, &free_pos_count, size_len);
    std::memcpy(bytes.data()+size_len+size_len, &end_pos, size_len);
    io.write_block(bytes, 0);

    // test_pos
    IO record_io(dir+"/"+"test_pos.sdb");
    Bytes record_bytes = io.read_block(0);
    size_t record_free_pos_count = 0;
    size_t record_free_end_pos = 0;
    std::memcpy(&record_free_pos_count, record_bytes.data(), size_len);
    std::memcpy(&record_free_end_pos, record_bytes.data()+size_len, size_len);
    record_io.write_block(record_bytes, 0);
}

void bpt_test() {
    BpTree<int, DB::Type::Bytes> bpTree(get_table_property());
    Bytes bytes(12);
//    for (int j = 0; j < 10; ++j) {
//
//    }
    int key = 1;
    std::memcpy(bytes.data(), &key, 4);
    std::memcpy(bytes.data()+4, std::string("fffffffff").data(), 8);
    for (int j = 0; j < 1000; ++j) {
        bpTree.insert(j, bytes);
    }
    bpTree.print();
}

void io_test(){
    auto dir = DB::Function::get_db_file_dir_path();
    IO io(dir+"/"+"test_pos.sdb");
    Bytes bytes(BLOCK_SIZE);
    for (int i = 0; i < 100; ++i) {
        bytes[i] = 'a';
    }
    io.write_block(bytes, 3);
    bytes = io.read_block(1);
    std::cout << bytes.data() << std::endl;
}
