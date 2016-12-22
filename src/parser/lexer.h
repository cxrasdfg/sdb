#ifndef PARSER_LEXER_HPP
#define PARSER_LEXER_HPP

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <functional>

typedef std::function<std::pair<std::string, int>(char)> fa_function_type;

class Lexer{
public:
    Lexer(){}
    Lexer(const Lexer &lexer){*this=lexer;}
    Lexer(Lexer &&lexer){*this=lexer;}
    const Lexer &operator=(const Lexer &lexer);
    const Lexer &operator=(Lexer &&lexer);

    auto tokenize(const std::string &str);
    std::pair<std::string, char> tokenize_char(char ch);

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
    const std::vector<fa_function_type> fa_function;
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
    auto token = std::pair<std::string, std::string>();
    auto vfa = this->fa_function;
    for (auto ch : str) {
        std::vector<fa_function_type> tem_vfa;
        for(auto &f: vfa){
            auto ret = f(ch);
            if (ret.second){
                tem_vfa.push_back(f);
            }
        }
    }
    return this->tokens;
}

#endif
