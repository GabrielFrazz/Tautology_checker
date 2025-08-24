#include "ast.h"
#include <iostream>
#include <algorithm>

void ASTNode::collectVariables(std::vector<std::string>& variables) const {
    if (type == NodeType::VARIABLE) {
        if (std::find(variables.begin(), variables.end(), value) == variables.end()) {
            variables.push_back(value);
        }
    }
    
    if (left) {
        left->collectVariables(variables);
    }
    if (right) {
        right->collectVariables(variables);
    }
}

void ASTNode::print(int depth) const {
    for (int i = 0; i < depth; ++i) {
        std::cout << "  ";
    }
    
    switch (type) {
        case NodeType::VARIABLE:
            std::cout << "VAR: " << value << std::endl;
            break;
        case NodeType::NOT:
            std::cout << "NOT" << std::endl;
            break;
        case NodeType::AND:
            std::cout << "AND" << std::endl;
            break;
        case NodeType::OR:
            std::cout << "OR" << std::endl;
            break;
        case NodeType::IMPLIES:
            std::cout << "IMPLIES" << std::endl;
            break;
    }
    
    if (left) {
        left->print(depth + 1);
    }
    if (right) {
        right->print(depth + 1);
    }
}