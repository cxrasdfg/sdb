#include <utility>
#include <string>
#include <fstream>
#include <iostream>

#include "ast.h"

// ========= Ast ========
void Ast::output_graphviz(const std::string &filename)const{
    std::ofstream out(filename);
    if (!out.is_open()){
        std::cout << filename << " not found!" << std::endl;
        exit(1);
    }
    out << "digraph Ast{\n";
    auto str = get_graphviz(root, 0, "");
    std::cout << str << std::endl;
    out.write(str.c_str(), str.length());
    out << "}";
}

std::string Ast::get_graphviz(std::shared_ptr<AstNode> ptr, int num, const std::string &p_name)const{
    if (ptr == nullptr)
        return "";
    std::string str;
    std::string current_name = p_name+"_"+ptr->name+std::to_string(num);
    str += "\""+current_name+"\"";
    str += " [label=\""+ptr->name+"\"];\n";
    int ch_num = 0;
    for (auto p :ptr->children) {
        str += "\""+current_name+"\"";
        str += "->\""+current_name+"_"+p->name+std::to_string(ch_num)+"\"\n";
        str += get_graphviz(p, ch_num, current_name)+"\n";
        ch_num++;
    }
    return str;
}
