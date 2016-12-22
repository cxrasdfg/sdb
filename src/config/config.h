#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

class Config{
public:
    boost::filesystem::path get_dir_path();
    boost::property_tree::ptree get_ptree_node(const std::string &filename);
};

class LexerConfig: public Config{
public:
    std::vector<std::string> get_category_list(const std::string &category_name);
    std::vector<std::string> get_reserved_list(){return this->get_category_list("reserved"); }
    std::vector<std::string> get_special_char_list(){return this->get_category_list("special_char"); }

private:
    std::string filename = "lexer.json";
};

#endif /* CONFIG_H */
