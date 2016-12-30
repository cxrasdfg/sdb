#include <iostream>
#include <tuple>
#include <memory>
#include <string>
#include <algorithm>
#include <iterator>

#include "lexer.h"
#include "ast.h"

#include "parser.h"

// ========== Parser function =========
std::shared_ptr<AstNode> Parser::parsing(const Parser::tokensType &tokens){
    std::cout << "parsing begin" << std::endl;
    is_r_to_deep();
    return statement_list_processing(tokens.cbegin(), tokens.cend());
}

std::shared_ptr<AstNode> Parser::statement_list_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();

    auto tokens = std::make_pair("statement_list_node", "statement_list_node");
    std::vector<std::shared_ptr<AstNode>> vec_ptr;
    auto split_iter = std::find(beg, end, ";");
    if (beg == end){
        vec_ptr.push_back(statement_processing(beg, split_iter));
        return std::make_shared<AstNode>(tokens, vec_ptr);
    }
    vec_ptr.push_back(statement_processing(beg, split_iter));
    auto statement_list_ptr = statement_processing(split_iter+1, end);
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

std::vector<std::shared_ptr<AstNode>> Parser::create_table_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
}

std::vector<std::shared_ptr<AstNode>> Parser::create_view_processing(tokensIterType beg, tokensIterType end){
    std::cout << "statement_list_processing begin" << std::endl;
    is_r_to_deep();
}
