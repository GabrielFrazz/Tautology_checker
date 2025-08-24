#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <vector>

enum class NodeType {
    VARIABLE,
    NOT,        
    AND,       
    OR,        
    IMPLIES     
};

class ASTNode {
public:
    NodeType type;
    std::string value;  
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
    
    ASTNode(NodeType t) : type(t), left(nullptr), right(nullptr) {}
    ASTNode(NodeType t, const std::string& val) : type(t), value(val), left(nullptr), right(nullptr) {}
    
    bool isLeaf() const { return left == nullptr && right == nullptr; }
    bool isUnary() const { return type == NodeType::NOT; }
    bool isBinary() const { return type == NodeType::AND || type == NodeType::OR || type == NodeType::IMPLIES; }
    

    void collectVariables(std::vector<std::string>& variables) const;
    

    void print(int depth = 0) const;
};

#endif