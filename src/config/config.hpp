#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <list>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

class Config{
public:
    Config(){}

    boost::filesystem::path get_dir_path();
    std::string get_cfg_text(const std::string &filename);
};

boost::filesystem::path Config::get_dir_path(){
    namespace bf = boost::filesystem;
    return bf::path(__FILE__).parent_path();
}

std::string Config::get_cfg_text(const std::string &filename){
    namespace bf = boost::filesystem;
    bf::path file_path(filename);
    std::fstream fs((this->get_dir_path() / file_path).generic_string());
    std::stringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

#endif /* CONFIG_H */
