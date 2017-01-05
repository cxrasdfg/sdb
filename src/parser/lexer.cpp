#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "../config/config.h"

#include "lexer.h"
// --------- Lexer Function --------

const Lexer &Lexer::operator=(const Lexer &lexer){
    if (this == &lexer){
        return *this;
    }
    LexerConfig cfg;
    this->tokens = lexer.tokens;
    return *this;
}

const Lexer &Lexer::operator=(Lexer &&lexer){
    return lexer;
}

bool Lexer::is_punctuation_char(char ch){
    auto char_set = cfg.get_punctuation_set();
    return char_set.find(ch) != char_set.end();
}

bool Lexer::is_reserved_word(const std::string &str){
    auto str_set = cfg.get_reserved_set();
    return str_set.find(str) != str_set.end();
}

bool Lexer::is_type_word(const std::string &str){
    auto str_set = cfg.get_type_set();
    return str_set.find(str) != str_set.end();
}

std::vector<std::pair<std::string, std::string>> Lexer::tokenize(const std::string &str){
    this->iter = str.cbegin();
    this->iter_end = str.cend();
    while (!is_end()) {
        std::pair<std::string, std::string> token;
        auto ch = get_char();
        if (ch == '_' || std::isalpha(ch))
            token = identifier_process();
        else if (is_punctuation_char(ch))
            token = punctuation_procerss();
        else if (ch > '0' && ch <'9')
            token = number_process();
        else if (ch == '\"' || ch == '\'')
            token = string_process();
        else if (ch == '-'){
            token = minus_and_comment_process();
            if (token.first.empty())
                continue;
        }
        else if (ch == '/'){
            token = div_and_comment_process();
            if (token.first.empty())
                continue;
        }
        else if (std::isspace(ch)){
            next_char();
            continue;
        } else {
            return tokens;
        }
        tokens.push_back(token);
    }
    return tokens;
}

// 处理标识符
std::pair<std::string, std::string> Lexer::identifier_process(){
    std::pair<std::string, std::string> ret;
    std::string word;
    auto ch = get_char();
    while (!is_end() && (std::isalpha(ch) || std::isalnum(ch) || ch == '_')){
        word += ch;
        next_char();
        ch = get_char();
    }
    ret.first = word;
    std::string word_lower = word;
    boost::algorithm::to_lower(word_lower);
    if (is_reserved_word(word_lower)){
        ret.second = "reserved_word";
    } else if (is_type_word(word_lower)) {
        ret.second = "type";
    } else {
        ret.second = "identifier";
    }
    return ret;
}

// 处理特殊符
std::pair<std::string, std::string> Lexer::punctuation_procerss(){
    std::string word = boost::lexical_cast<std::string>(get_char());
    next_char();
    return std::make_pair(word, word);
}

std::pair<std::string, std::string> Lexer::number_process(){
    std::string word;
    std::string category = "int";
    while (!is_end()){
        if (std::isalnum(get_char())){
            word += get_char();
            next_char();
        } else if (get_char() == '.') {
            next_char();
            word = '.' + number_float_process();
            category = "float";
        } else {
            break;
        }
    }
    return std::make_pair(word, category);
}

std::string Lexer::number_float_process(){
    std::string word;
    while (!is_end()){
        if (std::isalnum(get_char())){
            word += get_char();
            next_char();
        } else {
            std::cout << "float Error" << std::endl;
            return word;
        }
    }
    return word;
}

std::pair<std::string, std::string> Lexer::string_process(){
    std::string word;
    char rpair = get_char();
    next_char();
    while (!is_end() && get_char() != rpair){
        word += get_char();
        next_char();
    }
    next_char();
    return std::make_pair(word, "string");
}

std::pair<std::string, std::string> Lexer::div_and_comment_process(){
    std::pair<std::string, std::string> token;
    next_char();
    if (iter!=iter_end && get_char() != '*')
        return std::make_pair("/", "/");
    next_char();
    while (!is_end()) {
        if (get_char() == '*'){ 
            if ((iter+1) != iter_end && *(iter+1) == '/')
                iter += 2;
                return token;
        }
        next_char();
    }
    return token;
}

std::pair<std::string, std::string> Lexer::minus_and_comment_process(){
    next_char();
    if (iter!=iter_end && get_char() != '-')
        return std::make_pair("-", "-");
    else 
        while (iter!=iter_end && get_char()!='\n')
            next_char();
    next_char();
    return std::make_pair("", "");
}
