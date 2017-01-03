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
    Config(const std::string &fn):filename(fn){}
    boost::filesystem::path get_dir_path();
    boost::property_tree::ptree get_ptree_node();
    boost::property_tree::ptree get_ptree_node(const std::string &filename);
    std::unordered_set<std::string> get_str_set(const std::string &category_name);
    std::unordered_set<std::string> get_str_set(const std::string &filename, const std::string &category_name);

protected:
    std::string filename;
};

class LexerConfig: public Config{
public:
    LexerConfig():Config("lexer.json"){}
    std::unordered_set<std::string> get_reserved_set();
    std::unordered_set<std::string> get_type_set();
    std::unordered_set<char> get_punctuation_set();
};

#endif /* CONFIG_H */
