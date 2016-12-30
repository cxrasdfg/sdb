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

// ========== Parser function =========
std::shared_ptr<AstNode> Parser::parsing(const std::string &str){
    std::cout << "parsing begin" << std::endl;
    is_r_to_deep();

    Lexer lexer;
    auto tokens = lexer.tokenize(str);
    using namespace std::placeholders;
    return list_processing(tokens.cbegin(), tokens.cend(), ";", 
            std::bind(&Parser::statement_processing, this, _1, _2));
}

std::shared_ptr<AstNode>
Parser::list_processing(tokensIterType beg, 
        tokensIterType end,
        std::string sep, 
        std::function<std::shared_ptr<AstNode>(tokensIterType, tokensIterType)> fn){
    std::cout << "list_processing begin" << std::endl;
    is_r_to_deep();

    auto tokens = std::make_pair("list_node", "statement_list_node");
    std::vector<std::shared_ptr<AstNode>> vec_ptr;
    auto split_iter = std::find(beg, end, sep);
    if (beg == end){
        vec_ptr.push_back(fn(beg, split_iter));
        return std::make_shared<AstNode>(tokens, vec_ptr);
    }
    vec_ptr.push_back(fn(beg, split_iter));
    using namespace std::placeholders;
    auto statement_list_ptr = list_processing(split_iter+1, end, 
            ";", std::bind(&Parser::statement_processing, this, _1, _2));
    auto c_beg = statement_list_ptr->children.cbegin();
    auto c_end = statement_list_ptr->children.cend();
    vec_ptr.insert(vec_ptr.end(), c_beg, c_end);
    
    return std::make_shared<AstNode>(tokens, vec_ptr);
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
    auto token = std::make_pair("statement", tem_name+create_object);
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

    auto table_name_token = std::make_pair("table_name", beg->first);
    auto table_name_node = std::make_shared<AstNode>(table_name_token, nullptr);
    vec_ptr.push_back(table_name_node);
    auto column_list_ptr = column_def_list_processing(beg+2, end-1);

    auto column_list_token = std::make_pair("column_list", "column_list");
    auto column_list_node = std::make_shared<AstNode>(column_list_token, column_list_ptr);
    vec_ptr.push_back(column_list_node);
    return vec_ptr;
}

std::vector<std::shared_ptr<AstNode>> 
Parser::column_def_list_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
    

}

std::shared_ptr<AstNode> 
Parser::column_def_processing(tokensIterType beg, tokensIterType end){

}

std::vector<std::shared_ptr<AstNode>> Parser::create_view_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
}
