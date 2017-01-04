#ifndef AST_H
#define AST_H

#include <memory>
#include <utility>
#include <vector>

struct AstNode{
public:
    std::pair<std::string, std::string> token;
    std::vector<std::shared_ptr<AstNode>> children;

    AstNode (){}
    AstNode(const std::pair<std::string, std::string> &t, std::vector<std::shared_ptr<AstNode>> c)
        :token(t), children(c){}

    std::string get_node_dot()const;
};

class Ast{
public:
    Ast():root(nullptr){}
    Ast(std::shared_ptr<AstNode> r):root(r){}
    
    void output_graphviz(const std::string &filename)const;
    std::string get_graphviz(std::shared_ptr<AstNode> ptr)const;

private:
    std::shared_ptr<AstNode> root;
};

#endif /* AST_H */
