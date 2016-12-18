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
    Lexer(){}
    Lexer(const Lexer &lexer){*this=lexer;}
    Lexer(Lexer &&lexer){*this=lexer;}
    const Lexer &operator=(const Lexer &lexer);
    const Lexer &operator=(Lexer &&lexer);

    auto tokenize(const std::string &str);

private:
    std::vector<std::pair<std::string, std::string>> tokens;
    const std::unordered_set<char> char_set = {
        '*', ',', ';', '(', ')', '[', ']', '<', '>',
        '=', '&'
    };
    const std::unordered_set<std::string> reserved_word = {
        "abs", "and", "array", "as", "all",
        "select", "from", "where", "with", "insert"
    };
};

// --------- Function --------

const Lexer &Lexer::operator=(const Lexer &lexer){
    if (this == &lexer){
        return *this;
    }
    this->tokens = lexer.tokens;
    return *this;
}

const Lexer &Lexer::operator=(Lexer &&lexer){
    return lexer;
}

auto Lexer::tokenize(const std::string &str){
    auto iter = str.cbegin();
    auto token = std::pair<std::string, std::string>();
    while (iter != str.cend()){
        if (this->char_set.find(*iter) == this->char_set.cend()){
            token.first = boost::lexical_cast<std::string>(*iter);
            token.second = boost::lexical_cast<std::string>(*iter);
        }
        this->tokens.push_back(token);
    }
    return this->tokens;
}

#endif
