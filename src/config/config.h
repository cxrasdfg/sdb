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
    boost::filesystem::path get_dir_path();
    boost::property_tree::ptree get_ptree_node(const std::string &filename);
};

class LexerConfig: public Config{
public:
    std::unordered_set<std::string> get_category_set(const std::string &category_name);
    std::unordered_set<std::string> get_reserved_set(){return this->get_category_set("reserved"); }
    std::unordered_set<char> get_punctuation_set();

private:
    std::string filename = "lexer.json";
};

#endif /* CONFIG_H */
