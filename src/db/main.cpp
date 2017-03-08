#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <cstring>
#include <ctime>
#include <functional>
#include <cppformat/format.h>

#include "table.h"
#include "util.h"
#include "bpTree.h"

using std::cout;
using std::endl;
using DB::Type::Bytes;
using DB::Type::size_t;
using DB::Const::BLOCK_SIZE;

DB::Type::TableProperty get_table_property();
void bpt_test();
void io_test();
void table_init_test();

int main(void) {
    clock_t start = clock();
//    table_init();
//    io_test();
    table_init_test();
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

void bpt_test() {
    auto property = get_table_property();
    BpTree<int, DB::Type::Bytes> bpTree(property);
    Bytes bytes(12);
//    for (int j = 0; j < 10; ++j) {
//
//    }
    int key = 1;
    std::memcpy(bytes.data(), &key, 4);
    std::memcpy(bytes.data()+4, std::string("fffffffff").data(), 8);
    // insert test
    for (int j = 0; j < 1000; j += 2) {
        bpTree.insert(j, bytes);
    }
    bpTree.print();
    for (int j = 1; j < 1000; j += 2) {
        bpTree.insert(j, bytes);
    }
    // find test
    bytes = bpTree.find(999);
    std::memcpy(&key, bytes.data(), sizeof(int));
    cout << "key:" << key << endl;
    cout << "data:" << std::string(bytes.data()+4, bytes.data()+bytes.size()) << std::endl;
    Record::tuple_op(property, bytes, [](auto x){cout<<x<<" ";});
    std::cout << endl;
//     remove test
    for (int j = 0; j < 1000; j += 2) {
        bpTree.remove(j);
    }
    bpTree.print();
    for (int j = 1000; j < 1500; j += 2) {
        bpTree.insert(j, bytes);
    }
    for (int j = 1; j < 1000; j += 2) {
        bpTree.remove(j);
    }
    bpTree.print();
}

void io_test(){
    // test read/write block
    IO block_io("test_meta_record.sdb");
    Bytes bytes(BLOCK_SIZE);
    for (int i = 0; i < 100; ++i) {
        bytes[i] = 'a';
    }
    block_io.write_block(bytes, 0);
    bytes = block_io.read_block(0);
    std::cout << bytes.data() << std::endl;
    // test read/write file
    IO file_io("test_meta_record.sdb");
    bytes.clear();
    for (char j = 'a'; j < 'z'; ++j) {
        bytes.push_back(j);
    }
    file_io.write_file(bytes);
    bytes = file_io.read_file();
    DB::Function::bytes_print(bytes);
    // test create/delete file
    IO::create_file("test_create.sdb");
    IO::delete_file("test_create.sdb");
}

void table_init_test(){
    Table::create_table(get_table_property());
    Table table("test");
}
