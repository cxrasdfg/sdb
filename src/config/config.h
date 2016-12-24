#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <unordered_set>

class Config{
public:
    Config(){}
    Config(const std::string &fn):pt(get_ptree_node(fn)){}
    boost::filesystem::path get_dir_path();
    boost::property_tree::ptree get_ptree_node(const std::string &filename);

protected:
    boost::property_tree::ptree pt;
};

class LexerConfig: public Config{
public:
    LexerConfig():Config("lexer.json"){}
    std::unordered_set<std::string> get_reserved_set();
    std::unordered_set<char> get_punctuation_set();
};

#endif /* CONFIG_H */
