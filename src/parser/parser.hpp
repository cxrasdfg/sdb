//
// Created by sven on 16-12-12.
//

#ifndef PARSER_PARSER_HPP
#define PARSER_PARSER_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <boost/lexical_cast.hpp>

class Parser{
public:
	auto parsing(const std::string &str);
};

auto Parser::parsing(const std::string &str){
	return 1;
}

#endif //PARSER_PARSER_HPP
