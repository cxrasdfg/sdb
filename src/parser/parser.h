//
// Created by sven on 16-12-12.
//

#ifndef PARSER_PARSER_HPP
#define PARSER_PARSER_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <boost/lexical_cast.hpp>
#include <memory>

#include "ast.h"

class Parser{
public:
    // === Type ===
    using tokenType = std::pair<std::string, std::string>;
    using tokensType = std::vector<tokenType>;
    using tokensIterType = tokensType::const_iterator;

    Parser(){}

    std::shared_ptr<AstNode> parsing(const std::string &str);
    std::shared_ptr<AstNode> statement_processing(tokensIterType beg, tokensIterType end);

    std::shared_ptr<AstNode> create_processing(tokensIterType beg, tokensIterType end);
    std::vector<std::shared_ptr<AstNode>> create_table_processing(tokensIterType beg, tokensIterType end);
    std::vector<std::shared_ptr<AstNode>> column_def_list_processing(tokensIterType beg, tokensIterType end);
    std::shared_ptr<AstNode> column_def_processing(tokensIterType beg, tokensIterType end);
    std::vector<std::shared_ptr<AstNode>> column_def_context_list_processing(tokensIterType beg, tokensIterType end);

    std::vector<std::shared_ptr<AstNode>> create_view_processing(tokensIterType beg, tokensIterType end);

    std::shared_ptr<AstNode> select_processing(tokensIterType beg, tokensIterType end);
    std::shared_ptr<AstNode> insert_processing(tokensIterType beg, tokensIterType end);
    std::shared_ptr<AstNode> drop_processing(tokensIterType beg, tokensIterType end);

    std::vector<std::shared_ptr<AstNode>> list_processing(
            tokensIterType beg, 
            tokensIterType end,
            std::string sep, 
            std::function<std::shared_ptr<AstNode>(tokensIterType, tokensIterType)> fn);
    
    void is_r_to_deep(){
        r_count++;
        if (r_count > 100){
            std::cout << "recursion to deep" << std::endl;
            exit(1);
        }
    }
private:
    u_int r_count = 0;
};

#endif //PARSER_PARSER_HPP
