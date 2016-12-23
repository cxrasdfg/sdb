#include <list>
#include <string>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <unordered_set>

#include "config.h"

// ======== Config Function =========

boost::filesystem::path Config::get_dir_path(){
    namespace bf = boost::filesystem;
    return bf::path(__FILE__).parent_path();
}

boost::property_tree::ptree  Config::get_ptree_node(const std::string &filename){
    namespace bf = boost::filesystem;
    bf::path file_path(filename);
    std::fstream fs((this->get_dir_path() / file_path).generic_string());
    std::stringstream ss;
    ss << fs.rdbuf();
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    return pt;
}

// ======== LexerConfig Function =========

std::unordered_set<std::string> LexerConfig::get_category_set(const std::string &category_name){
    std::unordered_set<std::string> category_set;
    auto pt = this->get_ptree_node(this->filename);
    auto array_pt = pt.get_child(category_name);
    for (auto x : array_pt) {
        category_set.insert(x.second.data());
    }
    return category_set;
}

std::unordered_set<char> LexerConfig::get_punctuation_set(){
    auto pt = this->get_ptree_node(this->filename); 
    auto str_pt = pt.get_child("punctuation");
    std::unordered_set<char> ret;
    for (auto ch: str_pt.data()){
        ret.insert(ch);
    }
    return ret;
}
