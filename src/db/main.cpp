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
using DB::Const::INT_SIZE;
using DB::Type::Value;
using DB::Type::ColProperty;
using DB::Type::TableProperty;
using DB::Enum::ColType;
using DB::Type::Int;

DB::Type::TableProperty get_table_property();
void bpt_test();
void io_test();
void table_init_test();
void table_test();

int main(void) {
    clock_t start = clock();
//    io_test();
    Table::drop_table("test");
    table_init_test();
//    table_test();
    bpt_test();
    std::cout << "time:" << (double)((clock()-start))/CLOCKS_PER_SEC << std::endl;
    return 0;
}

DB::Type::TableProperty get_table_property(){
    std::vector<std::string> col_name_lst{"col_1", "col_2"};
    ColProperty::Tuple tuple("col_1", DB::Enum::INT, INT_SIZE);
    ColProperty::Tuple tuple2("col_2", DB::Enum::CHAR, 8);
    ColProperty col_property;
    col_property.tuple_lst.push_back(tuple);
    col_property.tuple_lst.push_back(tuple2);
    return TableProperty("test", "col_1", col_property);
}

void bpt_test() {
    auto property = get_table_property();
    BpTree bpTree(property);
    Bytes bytes(property.get_record_size());
//    for (int j = 0; j < 10; ++j) {
//
//    }
    Int key = 1;
    std::memcpy(bytes.data(), &key, INT_SIZE);
    std::memcpy(bytes.data()+INT_SIZE, std::string("fffffffff").data(), 8);
    // insert test
    for (Int j = 1; j < 1000; j += 2) {
        bpTree.insert(Value::make(DB::Enum::INT, j), bytes);
    }
    for (Int j = 100; j < 1000; j += 2) {
        bpTree.insert(Value::make(DB::Enum::INT, j), bytes);
    }
    bpTree.print();
    for (Int j = 100; j < 1000; j += 2) {
        bpTree.remove(Value::make(DB::Enum::INT, j));
    }
    bpTree.print();
    // key find test
    auto print_pos_lst = [](auto l){
        for (auto &&p : l) {
            cout << p << " ";
        }
        cout << endl;
    };
    DB::Type::PosList pos_lst = bpTree.find(Value::make(DB::Enum::INT, 11));
    print_pos_lst(pos_lst);
    // range find test
    pos_lst = bpTree.find(Value::make(DB::Enum::INT, 1), Value::make(DB::Enum::INT, 100));
    print_pos_lst(pos_lst);
    pos_lst = bpTree.find(Value::make(DB::Enum::INT, 100), true);
    print_pos_lst(pos_lst);
    pos_lst = bpTree.find(Value::make(DB::Enum::INT, 100), false);
    print_pos_lst(pos_lst);
//    for (int j = 1000; j < 1500; j += 2) {
//        bpTree.insert(j, bytes);
//    }
//    for (int j = 1; j < 1000; j += 2) {
//        bpTree.update(j);
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

void table_test(){
    //insert test
    Table table("test");
    std::vector<Value> vs{Value::str_to_value(DB::Enum::INT, "12"),
                          Value::str_to_value(DB::Enum::CHAR, "test_txt")};
    table.insert(vs);
    // make_predicate_func test
    auto e_f = Table::make_predicate_func("=");
    auto l_f = Table::make_predicate_func("<");
    cout << e_f(Value::make(DB::Enum::INT, 2), Value::make(DB::Enum::INT, 2)) << endl;
    cout << e_f(Value::make(DB::Enum::FLOAT, DB::Type::Float(2.0)), Value::make(DB::Enum::FLOAT, DB::Type::Float(2.1))) << endl;
    cout << e_f(Value::make(DB::Enum::CHAR, std::string("asf")), Value::make(DB::Enum::CHAR, std::string("asf"))) << endl;
    cout << l_f(Value::make(DB::Enum::INT, 1), Value::make(DB::Enum::INT, 2)) << endl;
    cout << l_f(Value::make(DB::Enum::FLOAT, DB::Type::Float(1.0)), Value::make(DB::Enum::FLOAT, DB::Type::Float(2.1))) << endl;
    cout << l_f(Value::make(DB::Enum::CHAR, std::string("asf")), Value::make(DB::Enum::CHAR, std::string("asf"))) << endl;
    // make_predicate_func test
    auto a_f = Table::make_op_func("+");
    cout << a_f(Value::make(DB::Enum::INT, 2),
                Value::make(DB::Enum::INT, 2)).get_string() << endl;
    cout << a_f(Value::make(DB::Enum::FLOAT, DB::Type::Float(2.0)),
                Value::make(DB::Enum::FLOAT, DB::Type::Float(2.1))).get_string() << endl;
}
