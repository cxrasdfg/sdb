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
    using tokenVecType = std::vector<tokenType>;
    using tokenVecIterType = tokenVecType::const_iterator;
    using nodePtrType = std::shared_ptr<AstNode>;
    using nodePtrVecType = std::vector<std::shared_ptr<AstNode>>;

    Parser(){}

    // === Pasring ===
    nodePtrType parsing(const std::string &str);
    nodePtrVecType statement_list_processing();
    nodePtrType statement_processing();

    nodePtrType create_processing();
    nodePtrVecType create_table_processing();
    nodePtrVecType col_def_list_processing();
    nodePtrType col_def_processing();
    nodePtrVecType col_def_context_list_processing();
    nodePtrType col_type_def();
    nodePtrType col_not_null_def();

    nodePtrVecType create_view_processing();

    nodePtrType select_processing();
    nodePtrType insert_processing();
    nodePtrType drop_processing();

    // === error ===
    void print_error(std::string str);

    // === debug ===
    void is_r_to_deep(std::string str);

private:
    u_int r_count = 0;
    tokenVecType::const_iterator iter;
    tokenVecType::const_iterator iter_end;
};

#endif //PARSER_PARSER_HPP
