#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../common/ast.h"
#include <unordered_map>
#include <stack>

class Evaluator {
private:
    std::unordered_map<std::string, bool> assignment;
    
    bool evaluateIterative(std::shared_ptr<ASTNode> root);
    
    bool evaluateRecursive(std::shared_ptr<ASTNode> node);
    
public:
    void setAssignment(const std::unordered_map<std::string, bool>& assignment);
    
    bool evaluate(std::shared_ptr<ASTNode> root);
    
    bool isTautology(std::shared_ptr<ASTNode> root, const std::vector<std::string>& variables);
    
    struct Metrics {
        long long evaluations;
        long long stackOperations;
        size_t maxStackSize;
        
        Metrics() : evaluations(0), stackOperations(0), maxStackSize(0) {}
    };
    
    Metrics metrics;
    void resetMetrics();
};

#endif
