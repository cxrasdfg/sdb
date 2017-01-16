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
#include <memory>
#include <unordered_set>

#include "ast.h"
#include "type.h"

class Parser{
public:
    Parser(){}
    // === Pasring ===
    Ast parsing(const std::string &str);
    ParserType::nodePtrVecType statement_list_processing();
    ParserType::nodePtrType statement_processing();

    // === create
    ParserType::nodePtrType create_processing();
    // create_table
    ParserType::nodePtrVecType create_table_processing();
    ParserType::nodePtrVecType col_def_list_processing();
    ParserType::nodePtrType col_def_processing();
    ParserType::nodePtrVecType col_def_context_list_processing();
    ParserType::nodePtrType col_type_def();
    ParserType::nodePtrType col_not_null_def();
    ParserType::nodePtrType col_primary_def_processing();
    // get column name list node ptr
    ParserType::nodePtrVecType col_name_list_processing(const std::string &terminor);
    ParserType::nodePtrType col_foreign_def_processing();
    ParserType::nodePtrType col_check_def_processing();

    // create_view
    ParserType::nodePtrVecType create_view_processing();

    // === query
    ParserType::nodePtrType query_processing();
    ParserType::nodePtrType query_list_processing();
    ParserType::nodePtrType query_from_processing();
    ParserType::nodePtrType query_where_processing();

    ParserType::nodePtrType insert_processing();
    ParserType::nodePtrType drop_processing();

    // === predicate
    ParserType::nodePtrType predicate_processing();
    ParserType::nodePtrType predicate_or_processing();
    ParserType::nodePtrType predicate_and_processing();
    ParserType::nodePtrType predicate_or_dot_processing();

    // === about iter ===
    ParserType::tokenType next_token(){return *iter++;}
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
