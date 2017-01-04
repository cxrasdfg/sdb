#ifndef ParserType_H
#define ParserType_H

#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "ast.h"

namespace ParserType{
    using tokenType = std::pair<std::string, std::string>;
    using tokenVecType = std::vector<tokenType>;
    using tokenVecIterType = tokenVecType::const_iterator;
    using nodePtrType = std::shared_ptr<AstNode>;
    using nodePtrVecType = std::vector<std::shared_ptr<AstNode>>;
}


#endif /* ifndef ParserType_H */
