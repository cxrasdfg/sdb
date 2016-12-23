#include <iostream>
#include <cctype>
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include "../config/config.h"

#include "lexer.h"
// --------- Lexer Function --------

Lexer::Lexer(){
    LexerConfig cfg;
    this->punctuation_set = cfg.get_punctuation_set();
    this->reserved_word = cfg.get_reserved_set();
}

const Lexer &Lexer::operator=(const Lexer &lexer){
    if (this == &lexer){
        return *this;
    }
    LexerConfig cfg;
    this->punctuation_set = cfg.get_punctuation_set();
    this->reserved_word = cfg.get_reserved_set();
    this->tokens = lexer.tokens;
    return *this;
}

const Lexer &Lexer::operator=(Lexer &&lexer){
    return lexer;
}

std::vector<std::pair<std::string, std::string>> Lexer::tokenize(const std::string &str){
    std::cout << "tokenize begin" << std::endl;
    std::pair<std::string, std::string> token;

    this->iter = str.cbegin();
    this->iter_end = str.cend();
    while (iter != iter_end) {
        std::pair<std::string, std::string> ret;
        auto ch = *iter;
        if (ch == '_' || std::isalpha(ch))
            token = this->identifier_process();
        else if (this->is_specail_char_trigger(ch))
            token = this->punctuation_procerss();
        else if (ch > '0' && ch <'9')
            token = this->number_process();
        else if (ch == '\"' || ch == '\'')
            token = this->string_process();
        else
            iter++;
        tokens.push_back(token);
    }
    return tokens;
}

// 处理标识符
std::pair<std::string, std::string> Lexer::identifier_process(){
    std::cout << "identifier_process begin" << std::endl;
    std::pair<std::string, std::string> ret;
    std::string word;
    while (iter != iter_end && ((*iter == '_') || (*iter>'a' &&*iter<'z') || (*iter>'A' && *iter<'Z'))){
        word += *iter;
        iter++;
    }
    ret.first = word;
    //for (auto x : reserved_word) {
        //std::cout << x << std::endl;
    //}
    ret.second = reserved_word.find(word) == reserved_word.end() ? "identifier" : "reserved_word";
    return ret;
}

// 处理特殊符
std::pair<std::string, std::string> Lexer::punctuation_procerss(){
    std::cout << "punctuation_procerss begin" << std::endl;
    std::string word = boost::lexical_cast<std::string>(*iter);
    iter++;
    return std::make_pair(word, word);
}

std::pair<std::string, std::string> Lexer::number_process(){
    std::cout << "number_process begin" << std::endl;
    std::string word;
    std::string category = "int";
    while (iter != iter_end){
        if (std::isalnum(*iter)){
            word += *iter;
            iter++;
        } else if (*iter == '.') {
            iter++;
            word = '.' + number_float_process();
            category = "float";
        } else {
            break;
        }
    }
    return std::make_pair(word, category);
}

std::string Lexer::number_float_process(){
    std::cout << "number_float_process begin" << std::endl;
    std::string word;
    while (iter != iter_end){
        if (std::isalnum(*iter)){
            word += *iter;
            iter++;
        } else {
            std::cout << "float Error" << std::endl;
            return word;
        }
    }
    return word;
}

std::pair<std::string, std::string> Lexer::string_process(){
    std::cout << "string_process begin" << std::endl;
    std::string word;
    char rpair = *iter;
    iter++;
    while (iter != iter_end){
        char ch = *iter;
        if (ch == rpair) {
            word += rpair;
            break;
        }
        word += ch;
    }
    return std::make_pair(word, "string");
}
