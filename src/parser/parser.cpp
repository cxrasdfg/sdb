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
    std::cout << "parsing begin" << std::endl;
    is_r_to_deep();

    Lexer lexer;
    auto tokens = lexer.tokenize(str);
    iter = tokens.cbegin();
    iter_end = tokens.cend();
    auto vec_ptr = statement_list_processing();
    
    tokenType token("root", "root");
    return std::make_shared<AstNode>(token, vec_ptr);
}

Parser::nodePtrVecType Parser::statement_list_processing(){
    std::cout << "list_processing begin" << std::endl;
    is_r_to_deep();

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
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();

    auto statement_name = iter->first;
    iter++;
    if (statement_name == "select")
        return select_processing();
    else if (statement_name == "create")
        return create_processing();
    else
        exit(1);
}

Parser::nodePtrType Parser::create_processing(){
    std::cout << "create_processing begin" << std::endl;
    is_r_to_deep();

    nodePtrVecType ptr_vec;
    auto create_object = iter->first;
    iter++;
    std::string tem_name = "create_";
    tokenType token("statement", tem_name+create_object);
    if (create_object == "table"){
        ptr_vec = create_table_processing();
    } else if (create_object == "view") {
        ptr_vec = create_view_processing();
    } else {
        exit(1);
    }
    return std::make_shared<AstNode>(token, ptr_vec);
}

Parser::nodePtrVecType Parser::create_table_processing(){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
    
    nodePtrVecType ptr_vec;

    tokenType table_name_token("table_name", iter->first);
    iter++;
    auto table_name_node = std::make_shared<AstNode>(table_name_token, nullptr);
    ptr_vec.push_back(table_name_node);

    auto col_ptr_vec = col_def_list_processing();
    tokenType col_list_token("column_list", "column_list");
    ptr_vec.insert(ptr_vec.end(), col_ptr_vec.begin(), col_ptr_vec.end());
    return ptr_vec;
}

Parser::nodePtrVecType Parser::col_def_list_processing(){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();     

    nodePtrVecType ptr_vec;
    if (iter->first != "("){
        std::cout << "-(-" << std::endl;
        exit(1);
    } else if (iter->first != ")") {
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
    std::cout << "col_def_processing begin" << std::endl;
    is_r_to_deep();     

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
    auto col_name_node = std::make_shared<AstNode>(col_name, nullptr);
    ptr_vec.push_back(col_name_node);

    auto def_ptr_vec = col_def_context_list_processing();
    tokenType token("col_def", "column_def");
    ptr_vec.insert(ptr_vec.end(), def_ptr_vec.begin(), def_ptr_vec.end());
    return std::make_shared<AstNode>(token, ptr_vec);
}

Parser::nodePtrVecType Parser::col_def_context_list_processing(){
    ;
}
