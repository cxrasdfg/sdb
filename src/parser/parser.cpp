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
using namespace ParserType;

// ========== Parser function =========
// input -> statemant_list
Ast Parser::parsing(const std::string &str){
    is_r_to_deep("parsing");

    Lexer lexer;
    auto tokens = lexer.tokenize(str);
    iter = tokens.cbegin();
    iter_end = tokens.cend();
    auto ptr_vec = statement_list_processing();
    
    auto root = std::make_shared<AstNode>("root", "root", ptr_vec);
    return Ast(root);
}

// statement_list -> statement ";" statement_list
//                 | statement
nodePtrVecType Parser::statement_list_processing(){
    is_r_to_deep("statement_list_processing");

    nodePtrVecType ptr_vec;
    if (is_end()){
        return ptr_vec;
    }
    auto statement_node = statement_processing();
    ptr_vec.push_back(statement_node);
    auto sl_ptr_vec = statement_list_processing();
    ptr_vec.insert(ptr_vec.end(), sl_ptr_vec.begin(), sl_ptr_vec.end());
    return ptr_vec;
}

// statement -> "create" create
//            | "drop" drop
//            | "insert" insert
//            | "delete" delete
//            | "select" query
nodePtrType Parser::statement_processing() {
    is_r_to_deep("statement_processing");

    std::cout << get_token_name() << std::endl;
    auto statement_name = get_token_name();
    next_token();
    nodePtrType statement_node;
    if (statement_name == "select")
        statement_node = query_processing();
    else if (statement_name == "create")
        statement_node = create_processing();
    else 
        print_error(std::string("statement_name:")+statement_name+" not found!");

    if (get_token_name() == ";")
        next_token();
    return statement_node;
}

// create -> create_table
//         | create_view
nodePtrType Parser::create_processing(){
    is_r_to_deep("create_processing");

    nodePtrVecType ptr_vec;
    auto create_object = get_token_name();
    next_token();
    std::string tem_name = "create_";
    if (create_object == "table"){
        ptr_vec = create_table_processing();
    } else if (create_object == "view") {
        ;
    } else {
        print_error(std::string("not found this create_object:"+create_object));
    }
    std::cout << "count:" << ptr_vec.size() << std::endl;
    return std::make_shared<AstNode>(tem_name+create_object, "statement", ptr_vec);
}

nodePtrVecType Parser::create_table_processing(){
    is_r_to_deep("create_table_processing");
    
    nodePtrVecType ptr_vec;

    auto token_name = get_token_name();
    std::cout << "table_name:" << token_name << std::endl;
    next_token();

    auto table_name_node = std::make_shared<AstNode>(token_name, "table_name", nodePtrVecType());
    ptr_vec.push_back(table_name_node);

    if (get_token_name() != "("){
        print_error(std::string("the word not (:")+get_token_name());
    }
    next_token();
    auto col_ptr_vec = col_def_list_processing();
    auto col_list_ptr = std::make_shared<AstNode>("col_list", "col_list", col_ptr_vec);
    ptr_vec.push_back(col_list_ptr);
    next_token();
    return ptr_vec;
}

nodePtrVecType Parser::col_def_list_processing(){
    is_r_to_deep("col_def_list_processing");     

    nodePtrVecType ptr_vec;
    nodePtrType ptr;
    auto fst = get_token_name();
    if (fst == ")"){
        return ptr_vec;
    } else if (fst == "primary") {
        ptr = col_primary_def_processing();
    } else if (fst == "foreign"){
        ptr = col_foreign_def_processing();
    } else {
        ptr = col_def_processing();
    }
    ptr_vec.push_back(ptr);
    auto col_ptr_vec = col_def_list_processing();
    ptr_vec.insert(ptr_vec.end(), col_ptr_vec.begin(), col_ptr_vec.end());
    return ptr_vec;
}

nodePtrType Parser::col_def_processing(){
    is_r_to_deep("col_def_processing");

    std::cout << get_token_name() << std::endl;
    std::cout << get_token_category() << std::endl;
    if (is_end()){
        print_error("end");
    } else if (get_token_category() != "identifier"){
        print_error(get_token_name()+" is not identifier");
    }
    std::vector<std::shared_ptr<AstNode>> ptr_vec;
    auto col_name = get_token_name();
    auto col_name_node = std::make_shared<AstNode>(col_name, "col_name", nodePtrVecType());
    ptr_vec.push_back(col_name_node);
    next_token();

    auto def_ptr_vec = col_def_context_list_processing();
    auto def_list_ptr = std::make_shared<AstNode>("col_def_context", "col_def_context", def_ptr_vec);
    ptr_vec.push_back(def_list_ptr);
    return std::make_shared<AstNode>("col_def", "col_def", ptr_vec);
}

nodePtrVecType Parser::col_def_context_list_processing(){
    is_r_to_deep("col_def_context_list_processing");

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
        auto fst = get_token_name();
        std::cout << "fst:" << fst << std::endl;
        if (fst == ","){
            next_token();
            return ptr_vec;
        } else if (fst == ")"){
            return ptr_vec;
        } else if (type_def_set.find(fst) != type_def_set.cend()){
            auto type_node_ptr = col_type_def();
            if (was_type_def)
                print_error("col type already def");
            ptr_vec.push_back(type_node_ptr);
            was_type_def = true;
        } else if (fst == "not") {
            if (was_not_null_def)
                print_error("col not_null already def");
            ptr_vec.push_back(col_not_null_def());
        } else {
            print_error("def not found");
        }
    }
    return ptr_vec;
}

nodePtrType Parser::col_type_def(){
    auto type_name = get_token_name();
    nodePtrType node_ptr = std::make_shared<AstNode>(type_name, "type_def", nodePtrVecType());
    next_token();
    if (type_name != "int" && type_name != "smallint"){
        auto fst = next_token().first;
        auto scd = next_token();
        auto trd = next_token().first;
        if (fst != "(" && scd.second != "identifier" && trd != ")"){
            print_error(type_name+" def error");
        }
        node_ptr->children.push_back(std::make_shared<AstNode>(scd.first, "type_length", nodePtrVecType()));
    }
    return node_ptr;
}

nodePtrType Parser::col_not_null_def(){
    next_token();
    auto token_name = get_token_name();
    if (token_name != "null")
        print_error("not null");
    next_token();
    return std::make_shared<AstNode>("not_null", "not_null", nodePtrVecType());
}

nodePtrType Parser::col_primary_def_processing(){
    is_r_to_deep("col_primary_def_processing");

    auto fst = next_token().first;
    auto scd = next_token().first;
    auto trd = next_token().first;
    if (fst != "primary" && scd != "key" && trd != "(")
        print_error("primary def error");
    auto ptr_vec = col_name_list_processing(")");
    next_token();
    return std::make_shared<AstNode>("primary_def", "primary_def", ptr_vec);
}

nodePtrVecType Parser::col_name_list_processing(const std::string &terminor){
    is_r_to_deep("col_name_list");

    nodePtrVecType ptr_vec;
    while (!is_end() && get_token_name() != terminor){
        if (get_token_name() == ","){
            next_token();
            continue;
        }
        auto ptr = std::make_shared<AstNode>(get_token_name(), "col_name", nodePtrVecType());
        ptr_vec.push_back(ptr);
        next_token();
    }
    next_token();
    return ptr_vec;
}

// column_def -> "foreign" "(" column_name_list ")" "references" table_name
nodePtrType Parser::col_foreign_def_processing(){
    is_r_to_deep("col_foreign_def_processing begin");
    
    nodePtrVecType ptr_vec;
    next_token();
    auto fst = next_token().first;
    if (is_end() || fst != "("){
        print_error("foreign def");
    }
    auto col_ptr_vec = col_name_list_processing(")");
    auto col_name_list_ptr = std::make_shared<AstNode>("col_name_list", "col_name_list", col_ptr_vec);
    fst = next_token().first;
    if (fst != "references"){
        print_error("need take a references table");
    }
    fst = next_token().first;
    std::cout << fst << std::endl;
    auto table_name_ptr = std::make_shared<AstNode>(fst, "table_name", nodePtrVecType());
    ptr_vec.push_back(table_name_ptr);
    ptr_vec.push_back(col_name_list_ptr);
    return std::make_shared<AstNode>("foreign_def", "foreign_def", ptr_vec);
}

// === query ===
// query -> "select" select_list "from" table_name_list "where" predicate
nodePtrType Parser::query_processing(){
    is_r_to_deep("query_processing begin");

    nodePtrVecType ptr_vec;

    // column name node
    auto col_name_list = col_name_list_processing("from");
    auto col_name_list_ptr = std::make_shared<AstNode>("select", "col_name_list", col_name_list);
    ptr_vec.push_back(col_name_list_ptr);

    // from node
    auto from_ptr = query_from_processing();
    ptr_vec.push_back(from_ptr);
    // where node
    if (get_token_name() == "where"){
        next_token();
        auto where_ptr = query_where_processing();
        ptr_vec.push_back(where_ptr);
    }
    return std::make_shared<AstNode>("query", "query", ptr_vec);
}

// input: table name list
nodePtrType Parser::query_from_processing(){
    is_r_to_deep("query_from_processing");

    nodePtrVecType ptr_vec;
    while (!is_end()){
        auto fst = get_token_name();
        if (fst == ","){
            next_token();
            continue;
        } else if (fst == "where" || fst == ";") {
            break;
        }
        auto ptr = std::make_shared<AstNode>(fst, "table_name", nodePtrVecType());
        ptr_vec.push_back(ptr);
        next_token();
    }
    return std::make_shared<AstNode>("from", "table_name_list", ptr_vec);
}

nodePtrType Parser::query_where_processing(){
    is_r_to_deep("query_where_processing begin");

    auto ptr = predicate_processing();
    nodePtrVecType ptr_vec;
    ptr_vec.push_back(ptr);
    return std::make_shared<AstNode>("where", "where", ptr_vec);
}

nodePtrType Parser::predicate_processing(){
    is_r_to_deep("predicate_processing begin");
}

// ========== error processing =========
void Parser::print_error(std::string str){
    std::cout << "Error:" << str << std::endl;
    exit(1);
}

// ========== debug processing =========
void Parser::is_r_to_deep(std::string str){
    std::cout << "function: " << str << " begin!" << std::endl;
    r_count++;
    if (r_count > 100){
        std::cout << "recursion to deep" << std::endl;
        exit(1);
    }
}
