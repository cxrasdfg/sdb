#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <cstring>
#include <ctime>
#include <functional>
#include <cppformat/format.h>
#include <memory>

#include "table.h"
#include "util.h"
#include "bpTree.h"
#include "cache.h"

using std::cout;
using std::endl;
using DB::Type::Bytes;
using DB::Type::Pos;
using DB::Const::BLOCK_SIZE;
using DB::Const::INT_SIZE;
using DB::Type::Value;
using DB::Type::TupleProperty;
using DB::Type::TableProperty;
using DB::Enum::ColType;
using DB::Type::Int;

DB::Type::TableProperty get_table_property();
void bpt_test();
void io_test();
void table_init_test();
void table_test();
void record_test();
void cache_test();

int main(void) {
    clock_t start = clock();
//    io_test();
//    Table::drop_table("test");
//    table_init_test();
//    bpt_test();
//    table_test();
//r   record_test();
    cache_test();
    std::cout << "time:" << (double)((clock()-start))/CLOCKS_PER_SEC << std::endl;
    return 0;
}

DB::Type::TableProperty get_table_property(){
    std::vector<std::string> col_name_lst{"col_1", "col_2"};
    TupleProperty::ColProperty tuple("col_1", DB::Enum::INT, INT_SIZE);
    TupleProperty::ColProperty tuple2("col_2", DB::Enum::VARCHAR, 8);
    TupleProperty col_property;
    col_property.property_lst.push_back(tuple);
    col_property.property_lst.push_back(tuple2);
    return TableProperty("test", "col_1", col_property);
}

void bpt_test() {
    auto property = get_table_property();
    BpTree bpTree(property);
//    for (int j = 0; j < 10; ++j) {
//
//    }
    Value str_value = Value::make(ColType::VARCHAR, std::string("asd"));
    Bytes str_bytes = Record::value_to_bytes(str_value);
    // insert test
    for (Int j = 1; j < 1000; j += 1) {
        Value key_value = Value::make(ColType::INT, DB::Type::Int(j));
        Bytes key_bytes = Record::value_to_bytes(key_value);
        key_bytes.insert(key_bytes.end(), str_bytes.begin(), str_bytes.end());
        bpTree.insert(Value::make(DB::Enum::INT, j), key_bytes);
    }
    bpTree.print();
    for (Int j = 100; j < 1000; j += 2) {
        bpTree.remove(Value::make(DB::Enum::INT, j));
    }
    bpTree.print();
    // key get test
    auto print_pos_lst = [](auto l){
        for (auto &&p : l) {
            cout << p << " ";
        }
        cout << endl;
    };
    DB::Type::PosList pos_lst = bpTree.find(Value::make(DB::Enum::INT, DB::Type::Int(1)));
    print_pos_lst(pos_lst);
    // range get test
    pos_lst = bpTree.find(Value::make(DB::Enum::INT, 1), Value::make(DB::Enum::INT, 100));
    print_pos_lst(pos_lst);
    pos_lst = bpTree.find(Value::make(DB::Enum::INT, 100), true);
    print_pos_lst(pos_lst);
    pos_lst = bpTree.find(Value::make(DB::Enum::INT, 100), false);
    print_pos_lst(pos_lst);
    Record record(get_table_property());
    Value v1 = Value::make(DB::Enum::INT, DB::Type::Int(3));
    DB::Type::TupleLst tuple_lst = record.find("col_1", [v1](auto x){ return v1 < x;});
    tuple_lst.print();
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
    using DB::Function::get_bvfunc;
    using DB::Enum::INT;
    //insert test
    Table table("test");
    // insert test
    for (Int i = 0; i < 1000; ++i) {
        DB::Type::Tuple tuple;
        Value v1 = Value::make(DB::Enum::INT, i);
        tuple.value_lst.push_back(v1);
        Value v2 = Value::make(DB::Enum::VARCHAR, std::string("asd"));
        tuple.value_lst.push_back(v2);
        table.insert(tuple);
    }
    // get test
    Value value = Value::make(DB::Enum::INT, Int(600));
    auto bvf = DB::Function::get_bvfunc(DB::Enum::LESS, value);
    DB::Type::TupleLst tuple_lst = table.find("col_1", bvf);
//    tuple_lst.print();
    // remove
    for (Int i = 0; i < 500; ++i) {
        value = Value::make(DB::Enum::INT, i);
        table.remove("col_1", value);
    }
    value = Value::make(DB::Enum::INT, Int(650));
    table.remove("col_1", get_bvfunc(DB::Enum::LESS, value));
    value = Value::make(DB::Enum::INT, Int(700));
//    BpTree bpTree(get_table_property());
//    bpTree.print();
//    auto str_value = Value::make(DB::Enum::VARCHAR, std::string("qwe"));
//    table.get("col_1", get_bvfunc(DB::Enum::LESS, value)).print();
    auto vvf = [](Value v){return Value::make(DB::Enum::VARCHAR, std::string("string"));};
    // update
    table.update("col_1", get_bvfunc(DB::Enum::LESS, value), "col_2", vvf);
    value = Value::make(DB::Enum::INT, Int(750));
    table.find("col_1", get_bvfunc(DB::Enum::LESS, value)).print();
}

void record_test(){
    Record record(get_table_property());
    Value v1 = Value::make(DB::Enum::INT, DB::Type::Int(3));
    Value v2 = Value::make(DB::Enum::VARCHAR, std::string("asd"));
    DB::Type::Tuple tuple;
    tuple.value_lst.push_back(v1);
    tuple.value_lst.push_back(v2);
    Bytes bytes = Record::tuple_to_bytes(tuple);
    for (int i = 0; i < 1000; ++i) {
        auto pos = record.insert_record(bytes);
        cout << pos << " ";
        if (i % 100 == 99) {
            record.remove_record(pos);
        }
    }

    DB::Type::TupleLst tuple_lst = record.find("col_1", [v1](auto x){ return x == v1;});
    for (auto &&tup : tuple_lst.tuple_lst) {
        cout << "[ ";
        for (auto &&value : tup.value_lst) {
            cout << value.get_string() << " ";
        }
        cout << "]" << endl;
    }
    cout << endl;
}

void cache_test() {
    Cache cache(4);
    auto file = "test";
    for (size_t i = 0; i < 4; ++i) {
        auto str = std::to_string(i);
        auto ptr = std::make_shared<Bytes>(Bytes(str.begin(), str.end()));
        cache.put(file, i, ptr);
    }
    for (size_t i = 0; i < 4; ++i) {
        auto data = cache.get(file, i);
        if (data) {
            cout << "get: " << i << ":" << data->data() << endl;
        }
    }
    for (size_t i = 0; i < 10; ++i) {
        auto str = std::to_string(i);
        auto ptr = std::make_shared<Bytes>(Bytes(str.begin(), str.end()));
        cache.put(file, i, ptr);
    }
    for (size_t i = 0; i < 10; ++i) {
        auto data = cache.get(file, i);
        if (data) {
            cout << "get: " << i << ":" << data->data() << endl;
        }
    }
}
