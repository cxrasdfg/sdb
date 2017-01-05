#ifndef AST_H
#define AST_H

#include <memory>
#include <utility>
#include <vector>

struct AstNode{
    std::string name;
    std::string category;
    std::string type;
    std::vector<std::shared_ptr<AstNode>> children;

    AstNode (){}
    AstNode (const std::string &n):name(n){}
    AstNode (const std::string &n, const std::vector<std::shared_ptr<AstNode>> &c)
        :name(n), children(c){}
    AstNode (const std::string &n, const std::string &ctg,
            const std::vector<std::shared_ptr<AstNode>> &c)
        :name(n), category(ctg), children(c){}
    AstNode (const std::string &n, const std::string &ctg, const std::string &t,
            const std::vector<std::shared_ptr<AstNode>> &c)
        :name(n), category(ctg), type(t), children(c){}

    std::string get_node_dot(int num)const;
};

class Ast{
public:
    Ast():root(nullptr){}
    Ast(std::shared_ptr<AstNode> r):root(r){}
    
    void output_graphviz(const std::string &filename)const;
    std::string get_graphviz(std::shared_ptr<AstNode> ptr, int num)const;

private:
    std::shared_ptr<AstNode> root;
};

#endif /* AST_H */
