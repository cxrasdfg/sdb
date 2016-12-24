#ifndef PARSER_LEXER_HPP
#define PARSER_LEXER_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <functional>


class Lexer{
public:
    Lexer();
    Lexer(const Lexer &lexer){*this=lexer;}
    Lexer(Lexer &&lexer){*this=lexer;}
    const Lexer &operator=(const Lexer &lexer);
    const Lexer &operator=(Lexer &&lexer);

    std::vector<std::pair<std::string, std::string>> tokenize(const std::string &str);

    // 初始状态触发函数
    bool is_specail_char_trigger(char ch){
        return this->punctuation_set.find(ch) != this->punctuation_set.cend();
    }
    
    // 状态处理函数
    std::pair<std::string, std::string> identifier_process();
    std::pair<std::string, std::string> punctuation_procerss();

    std::pair<std::string, std::string> number_process();
    std::string number_float_process();
    std::pair<std::string, std::string> string_process();

    // 注释部分
    std::pair<std::string, std::string> div_and_comment_process();
    std::pair<std::string, std::string> minus_and_comment_process();

private:
    // token集，lexer返回的值
    std::vector<std::pair<std::string, std::string>> tokens;

    // 特殊集合
    std::unordered_set<char> punctuation_set;
    std::unordered_set<std::string> reserved_word;

    std::string::const_iterator iter;
    std::string::const_iterator iter_end;
};

#endif
