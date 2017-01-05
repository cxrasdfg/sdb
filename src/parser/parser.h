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
#include "type.h"

class Parser{
public:
    Parser(){}
    // === Pasring ===
    Ast parsing(const std::string &str);
    ParserType::nodePtrVecType statement_list_processing();
    ParserType::nodePtrType statement_processing();

    ParserType::nodePtrType create_processing();
    ParserType::nodePtrVecType create_table_processing();
    ParserType::nodePtrVecType col_def_list_processing();
    ParserType::nodePtrType col_def_processing();
    ParserType::nodePtrVecType col_def_context_list_processing();
    ParserType::nodePtrType col_type_def();
    ParserType::nodePtrType col_not_null_def();

    ParserType::nodePtrVecType create_view_processing();

    ParserType::nodePtrType select_processing();
    ParserType::nodePtrType insert_processing();
    ParserType::nodePtrType drop_processing();

    // about iter
    void next_token(){iter++;}
    bool is_end()const{return iter == iter_end; }
    std::string get_token_name()const{return iter->first; }
    std::string get_token_category()const{return iter->second; }

    // === error ===
    void print_error(std::string str);

    // === debug ===
    void is_r_to_deep(std::string str);

private:
    u_int r_count = 0;
    ParserType::tokenVecType::const_iterator iter;
    ParserType::tokenVecType::const_iterator iter_end;
};

#endif //PARSER_PARSER_HPP
