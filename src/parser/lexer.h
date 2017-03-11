#ifndef PARSER_LEXER_HPP
#define PARSER_LEXER_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <functional>
#include "../config/config.h"


class Lexer{
public:
    Lexer(){}
    Lexer(const Lexer &lexer){*this=lexer;}
    const Lexer &operator=(const Lexer &lexer);

    // special set
    bool is_punctuation_char(char ch);
    bool is_reserved_word(const std::string &str);
    bool is_type_word(const std::string &str);
    
    // tokenozie
    std::vector<std::pair<std::string, std::string>> tokenize(const std::string &str);
    // 状态处理函数
    std::pair<std::string, std::string> identifier_process();
    std::pair<std::string, std::string> punctuation_procerss();

    // number
    std::pair<std::string, std::string> number_process();
    std::string number_float_process();
    // string
    std::pair<std::string, std::string> string_process();

    // 注释部分
    std::pair<std::string, std::string> div_and_comment_process();
    std::pair<std::string, std::string> minus_and_comment_process();

    // iter
    bool is_end()const{return iter == iter_end;}
    char get_char()const{return *iter;}
    char next_char(){return *iter++;}

private:
    // token集，lexer返回的值
    std::vector<std::pair<std::string, std::string>> tokens;
    // config
    LexerConfig cfg;
    std::string::const_iterator iter;
    std::string::const_iterator iter_end;
};

#endif
