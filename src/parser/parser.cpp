#include <iostream>
#include <tuple>
#include <memory>
#include <string>
#include <algorithm>
#include <iterator>
#include <functional>

#include "lexer.h"
#include "ast.h"

#include "parser.h"
// ========== Type or namespace =========

using std::placeholders::_1;
using std::placeholders::_2;

// ========== Parser function =========


Parser::nodePtrType Parser::parsing(const std::string &str){
    is_r_to_deep("parsing");

    Lexer lexer;
    auto tokens = lexer.tokenize(str);
    iter = tokens.cbegin();
    iter_end = tokens.cend();
    auto vec_ptr = statement_list_processing();
    
    tokenType token("root", "root");
    return std::make_shared<AstNode>(token, vec_ptr);
}

Parser::nodePtrVecType Parser::statement_list_processing(){
    is_r_to_deep("statement_list_processing");

    nodePtrVecType ptr_vec;
    if (iter == iter_end){
        return ptr_vec;
    }
    tokenType token("statement_list_node ", "statement_list_node");
    auto statement_node = statement_processing();
    ptr_vec.push_back(statement_node);
    auto vec_ptr = statement_list_processing();
    vec_ptr.insert(vec_ptr.end(), vec_ptr.begin(), vec_ptr.end());
    return vec_ptr;
}

Parser::nodePtrType Parser::statement_processing() {
    is_r_to_deep("statement_processing");

    auto statement_name = iter->first;
    iter++;
    if (statement_name == "select")
        //return select_processing();
        ;
    else if (statement_name == "create")
        return create_processing();
    else
        exit(1);
}

Parser::nodePtrType Parser::create_processing(){
    is_r_to_deep("create_processing");

    nodePtrVecType ptr_vec;
    auto create_object = iter->first;
    iter++;
    std::string tem_name = "create_";
    tokenType token("statement", tem_name+create_object);
    if (create_object == "table"){
        ptr_vec = create_table_processing();
    } else if (create_object == "view") {
        //ptr_vec = create_view_processing();
        ;
    } else {
        exit(1);
    }
    return std::make_shared<AstNode>(token, nodePtrVecType());
}

Parser::nodePtrVecType Parser::create_table_processing(){
    is_r_to_deep("create_table_processing");
    
    nodePtrVecType ptr_vec;

    tokenType table_name_token("table_name", iter->first);
    iter++;
    auto table_name_node = std::make_shared<AstNode>(table_name_token, ptr_vec);
    ptr_vec.push_back(table_name_node);

    auto col_ptr_vec = col_def_list_processing();
    tokenType col_list_token("column_list", "column_list");
    ptr_vec.insert(ptr_vec.end(), col_ptr_vec.begin(), col_ptr_vec.end());
    return ptr_vec;
}

Parser::nodePtrVecType Parser::col_def_list_processing(){
    is_r_to_deep("statement_list_processing");     

    nodePtrVecType ptr_vec;
    if (iter->first != "("){
        std::cout << "-(-" << std::endl;
        exit(1);
    }
    if (iter->first != ")") {
        return ptr_vec;
    }
    iter++;
    auto ptr = col_def_processing();
    tokenType token("col_def_list", "col_def_list");
    ptr_vec.push_back(ptr);
    auto col_ptr_vec = col_def_list_processing();
    ptr_vec.insert(ptr_vec.end(), col_ptr_vec.begin(), col_ptr_vec.end());
    return ptr_vec;
}

Parser::nodePtrType Parser::col_def_processing(){
    is_r_to_deep("col_def_processing");

    if (iter == iter_end){
        std::cout << "error" << std::endl;
        exit(1);
    } else if (iter->second != "indentifier"){
        std::cout << "col_name must be indentifier!" << std::endl;
        exit(1);
    }
    std::vector<std::shared_ptr<AstNode>> ptr_vec;
    auto col_name = iter->first;
    tokenType col_name_token("col_name", col_name);
    auto col_name_node = std::make_shared<AstNode>(col_name_token, nodePtrVecType());
    ptr_vec.push_back(col_name_node);

    auto def_ptr_vec = col_def_context_list_processing();
    tokenType token("col_def", "column_def");
    ptr_vec.insert(ptr_vec.end(), def_ptr_vec.begin(), def_ptr_vec.end());
    return std::make_shared<AstNode>(token, ptr_vec);
}

Parser::nodePtrVecType Parser::col_def_context_list_processing(){
    is_r_to_deep("col_def_list_processing");

    if (iter == iter_end){
        print_error("def list");
    }
    std::unordered_set<std::string> type_def_set = {
        "int", "char", "varchar", "float", "smallint"
    };
    bool was_type_def = false;
    bool was_not_null_def = false;
    nodePtrVecType ptr_vec;
    while (iter != iter_end){
        auto fst = iter->first;
        if (fst == ","){
            iter++;
            return ptr_vec;
        } else if (fst == ")"){
            return ptr_vec;
        } else if (type_def_set.find(fst) != type_def_set.cend()){
            auto type_node_ptr = col_type_def();
            if (was_type_def) print_error("col type already def");
            ptr_vec.push_back(type_node_ptr);
            iter++;
            was_type_def = true;
        } else if (fst == "not") {
            if (was_not_null_def) print_error("col not_null already def");
            ptr_vec.push_back(col_not_null_def());
        } else {
            print_error("def not fount");
        }
    }
    return ptr_vec;
}

Parser::nodePtrType Parser::col_type_def(){
    auto type_name = iter->first;
    iter++;
    tokenType type_def_token("", "type_def");
    if (type_name == "int" || type_name == "smallint"){
        type_def_token.first = type_name;
    }
    return std::make_shared<AstNode>(type_def_token, nodePtrVecType());
}

Parser::nodePtrType Parser::col_not_null_def(){
    iter++;
    if (iter->first == "null"){
        tokenType not_null_token("not_null", "not_null");
        return std::make_shared<AstNode>(not_null_token, nodePtrVecType());
    } else {
        print_error("not null");
    }
}

// ========== error processing =========
void Parser::print_error(std::string str){
    std::cout << "Error:" << str << std::endl;
    exit(1);
}

// ========== debug processing =========
void Parser::is_r_to_deep(std::string str){
    std::cout << "function:" << str << "begin" << std::endl;
    r_count++;
    if (r_count > 100){
        std::cout << "recursion to deep" << std::endl;
        exit(1);
    }
}
