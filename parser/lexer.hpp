#ifndef PARSER_LEXER_HPP
#define PARSER_LEXER_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>

#ifndef CHAR_SET
const std::unordered_set<char> CHAR_SET = {
    '*', ',', ';', '(', ')', '[', ']', '<', '>',
    '=', '&'
};
#endif

class Lexer{
public:
    Lexer():tokens(), char_set(CHAR_SET){}
    Lexer(const Lexer &lexer){*this=lexer;}
    Lexer(Lexer &&lexer){*this=lexer;}
    const Lexer &operator=(const Lexer &lexer);
    const Lexer &operator=(Lexer &&lexer);

    auto tokenize(const std::string &str);

private:
    std::vector<std::pair<std::string, std::string>> tokens;
    std::unordered_set<char> char_set;
};

// --------- Function --------

const Lexer &Lexer::operator=(const Lexer &lexer){
    if (this == &lexer){
        return *this;
    }
    this->tokens = lexer.tokens;
    this->char_set = lexer.char_set;
    return *this;
}

const Lexer &Lexer::operator=(Lexer &&lexer){
    return lexer;
}

auto Lexer::tokenize(const std::string &str){
    auto iter = str.cbegin();
    while (iter != str.cend()){
        ;
    }
}

#endif
