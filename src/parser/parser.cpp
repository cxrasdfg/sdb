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


std::shared_ptr<AstNode> Parser::parsing(const std::string &str){
    std::cout << "parsing begin" << std::endl;
    is_r_to_deep();

    Lexer lexer;
    auto tokens = lexer.tokenize(str);
    auto vec_ptr = list_processing(tokens.cbegin(), tokens.cend(), ";", 
            std::bind(&Parser::statement_processing, this, _1, _2));
    
    tokenType token("root", "root");
    return std::make_shared<AstNode>(token, vec_ptr);
}

// === 列表通用递归函数 ===
// + beg/end token stream的首尾迭代器
// + sep 递归分割符
// + fn  节点处理函数
std::vector<std::shared_ptr<AstNode>>
Parser::list_processing(tokensIterType beg, 
        tokensIterType end,
        std::string sep, 
        std::function<std::shared_ptr<AstNode>(tokensIterType, tokensIterType)> fn){
    std::cout << "list_processing begin" << std::endl;
    is_r_to_deep();

    tokenType token("list_node", "statement_list_node");
    std::vector<std::shared_ptr<AstNode>> vec_ptr;
    auto split_iter = std::find(beg, end, sep);
    vec_ptr.push_back(fn(beg, split_iter));
    if (split_iter == end){
        return vec_ptr;
    }
    auto statement_list_ptr = list_processing(split_iter+1, end, ";",
            std::bind(&Parser::statement_processing, this, _1, _2));
    auto c_beg = statement_list_ptr.cbegin();
    auto c_end = statement_list_ptr.cend();
    vec_ptr.insert(vec_ptr.end(), c_beg, c_end);
    return vec_ptr;
}

std::shared_ptr<AstNode> Parser::statement_processing(tokensIterType beg, tokensIterType end) {
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();

    auto statement_name = beg->first;
    if (statement_name == "select")
        return select_processing(beg+1, end);
    else if (statement_name == "create")
        return create_processing(beg+1, end);
    else
        exit(1);
}

std::shared_ptr<AstNode> Parser::create_processing(tokensIterType beg, tokensIterType end){
    std::cout << "create_processing begin" << std::endl;
    is_r_to_deep();

    std::vector<std::shared_ptr<AstNode>> vec_ptr;
    auto create_object = beg->first;
    std::string tem_name = "create_";
    tokenType token("statement", tem_name+create_object);
    if (create_object == "table"){
        auto column_prts = create_table_processing(beg+1, end);
        vec_ptr.insert(vec_ptr.end(), column_prts.cbegin(), column_prts.cend());
    } else if (create_object == "view") {
        auto column_prts = create_view_processing(beg+1, end);
        vec_ptr.insert(vec_ptr.end(), column_prts.cbegin(), column_prts.cend());
    } else {
        exit(1);
    }
    return std::make_shared<AstNode>(token, vec_ptr);
}

std::vector<std::shared_ptr<AstNode>> 
Parser::create_table_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
    
    if ((beg+1)->first!="(" && (end-1)->first!= ")"){
        std::cout << "column_list_context error" << std::endl;
        exit(1);
    }
    std::vector<std::shared_ptr<AstNode>> vec_ptr;

    tokenType table_name_token("table_name", beg->first);
    auto table_name_node = std::make_shared<AstNode>(table_name_token, nullptr);
    vec_ptr.push_back(table_name_node);
    auto column_list_ptr = column_def_list_processing(beg+2, end-1);

    tokenType column_list_token("column_list", "column_list");
    auto column_list_node = std::make_shared<AstNode>(column_list_token, column_list_ptr);
    vec_ptr.push_back(column_list_node);
    return vec_ptr;
}

std::vector<std::shared_ptr<AstNode>>
Parser::column_def_list_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
    
    return list_processing(beg, end, ",",
            std::bind(&Parser::column_def_processing, this, _1, _2));
}

std::shared_ptr<AstNode> 
Parser::column_def_processing(tokensIterType beg, tokensIterType end){
    if (beg->second != "indentifier"){
        std::cout << "column_name must be indentifier!" << std::endl;
        exit(1);
    }
    std::vector<std::shared_ptr<AstNode>> vec_ptr;

    auto column_name = beg->first;
    tokenType column_name_token("column_name", column_name);
    auto column_name_node = std::make_shared<AstNode>(column_name, nullptr);
    vec_ptr.push_back(column_name_node);

    auto def_ptrs = column_def_context_list_processing(beg+1, end);
    tokenType token("column_def", "column_def");
    vec_ptr.insert(vec_ptr.end(), def_ptrs.begin(), def_ptrs.end());
    return std::make_shared<AstNode>(token, vec_ptr);
}

std::vector<std::shared_ptr<AstNode>>
Parser::column_def_context_list_processing(tokensIterType beg, tokensIterType end){

}
std::vector<std::shared_ptr<AstNode>> Parser::create_view_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
}
