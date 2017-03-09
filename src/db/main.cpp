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
using DB::Type::Pos;
using DB::Const::BLOCK_SIZE;
using DB::Type::Value;
using DB::Enum::ColType;
using DB::Type::Int;

DB::Type::TableProperty get_table_property();
void bpt_test();
void io_test();
void table_init_test();

int main(void) {
    clock_t start = clock();
//    io_test();
    Table::drop_table("test");
    table_init_test();
    bpt_test();
    std::cout << "time:" << (double)((clock()-start))/CLOCKS_PER_SEC << std::endl;
    return 0;
}

DB::Type::TableProperty get_table_property(){
    std::vector<std::string> col_name_lst{"col_1", "col_2"};
    DB::Type::TableProperty::ColProperty map{
            {"col_1", std::make_pair(DB::Enum::INT, 8)},
            {"col_2", std::make_pair(DB::Enum::VARCHAR, 8)}
    };
    DB::Type::TableProperty tableProperty("test", "col_1", col_name_lst, map);
    return tableProperty;
}

void bpt_test() {
    auto property = get_table_property();
    BpTree bpTree(property);
    Bytes bytes(property.get_record_size());
//    for (int j = 0; j < 10; ++j) {
//
//    }
    Int key = 1;
    std::memcpy(bytes.data(), &key, 8);
    std::memcpy(bytes.data()+8, std::string("fffffffff").data(), 8);
    // insert test
    for (Int j = 0; j < 1000; j += 2) {
        bpTree.insert(Value(j), bytes);
    }
//    bpTree.print();
//    for (int j = 1; j < 1000; j += 2) {
//        bpTree.insert(j, bytes);
//    }
    // find test
    bpTree.print();
//    bytes = bpTree.find(999);
//    std::memcpy(&key, bytes.data(), sizeof(int));
//    Record::tuple_op(property, bytes, [](auto x){cout<<x<<" ";});
//    std::cout << endl;
//     remove test
//    for (int j = 0; j < 1000; j += 2) {
//        bpTree.remove(j);
//    }
//    bpTree.print();
//    for (int j = 1000; j < 1500; j += 2) {
//        bpTree.insert(j, bytes);
//    }
//    for (int j = 1; j < 1000; j += 2) {
//        bpTree.remove(j);
//    }
//    bpTree.print();
}

void io_test(){
    // test read/write block
    IO::delete_file("test_meta_record.sdb");
    IO::create_file("test_meta_record.sdb");
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
}
