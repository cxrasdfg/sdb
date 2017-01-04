#include <utility>
#include <string>
#include <fstream>
#include <iostream>

#include "ast.h"

// ========= AstNode ========

std::string AstNode::get_node_dot(int num)const{
    std::string str;
    return str+token.second+std::to_string(num)+"[label="+token.first+"];\n";
}

// ========= Ast ========
void Ast::output_graphviz(const std::string &filename)const{
    std::ofstream out(filename);
    if (!out.is_open()){
        std::cout << filename << " not found!" << std::endl;
        exit(1);
    }
    out << "digraph Ast{\n";
    auto str = get_graphviz(root, 0);
    std::cout << str << std::endl;
    out.write(str.c_str(), str.length());
    out << "}";
}

std::string Ast::get_graphviz(std::shared_ptr<AstNode> ptr, int num)const{
    if (ptr == nullptr)
        return "";
    auto str = ptr->get_node_dot(num);
    int ch_num = num;
    for (auto p :ptr->children) {
        str += ptr->token.second+std::to_string(num);
        str += "->"+p->token.second+std::to_string(ch_num)+"\n";
        str += get_graphviz(p, ch_num)+"\n";
        ch_num++;
    }
    return str;
}
