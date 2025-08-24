#include "evaluator.h"
#include "../common/utils.h"
#include <iostream>
#include <algorithm>

struct StackFrame {
    std::shared_ptr<ASTNode> node;
    bool visited;  // Para pós-ordem
    bool value;    // Para armazenar resultado
    
    StackFrame(std::shared_ptr<ASTNode> n) : node(n), visited(false), value(false) {}
};

void Evaluator::setAssignment(const std::unordered_map<std::string, bool>& assignment) {
    this->assignment = assignment;
}

bool Evaluator::evaluate(std::shared_ptr<ASTNode> root) {
    metrics.evaluations++;
    return evaluateIterative(root);
}

bool Evaluator::evaluateIterative(std::shared_ptr<ASTNode> root) {
    if (!root) return false;
    
    std::stack<StackFrame> evalStack;
    std::stack<bool> valueStack;
    
    evalStack.push(StackFrame(root));
    metrics.maxStackSize = std::max(metrics.maxStackSize, evalStack.size());
    
    while (!evalStack.empty()) {
        StackFrame& frame = evalStack.top();
        metrics.stackOperations++;
        
        if (frame.node->isLeaf()) {
            // Variável
            if (frame.node->type == NodeType::VARIABLE) {
                auto it = assignment.find(frame.node->value);
                if (it != assignment.end()) {
                    valueStack.push(it->second);
                } else {
                    throw std::runtime_error("Variável não encontrada: " + frame.node->value);
                }
            }
            evalStack.pop();
            
        } else if (!frame.visited) {
            // Primeira visita - empilha filhos
            frame.visited = true;
            
            if (frame.node->type == NodeType::NOT) {
                // Unário
                evalStack.push(StackFrame(frame.node->left));
            } else {
                // Binário - empilha na ordem inversa (direita primeiro)
                evalStack.push(StackFrame(frame.node->right));
                evalStack.push(StackFrame(frame.node->left));
            }
            
            metrics.maxStackSize = std::max(metrics.maxStackSize, evalStack.size());
            
        } else {
            // Segunda visita - processa operação
            bool result = false;
            
            switch (frame.node->type) {
                case NodeType::NOT: {
                    bool operand = valueStack.top();
                    valueStack.pop();
                    result = !operand;
                    break;
                }
                case NodeType::AND: {
                    bool right = valueStack.top(); valueStack.pop();
                    bool left = valueStack.top(); valueStack.pop();
                    result = left && right;
                    break;
                }
                case NodeType::OR: {
                    bool right = valueStack.top(); valueStack.pop();
                    bool left = valueStack.top(); valueStack.pop();
                    result = left || right;
                    break;
                }
                case NodeType::IMPLIES: {
                    bool right = valueStack.top(); valueStack.pop();
                    bool left = valueStack.top(); valueStack.pop();
                    result = !left || right; // p → q ≡ ¬p ∨ q
                    break;
                }
                default:
                    throw std::runtime_error("Tipo de nó desconhecido");
            }
            
            valueStack.push(result);
            evalStack.pop();
        }
    }
    
    if (valueStack.size() != 1) {
        throw std::runtime_error("Erro na avaliação: pilha de valores incorreta");
    }
    
    return valueStack.top();
}

bool Evaluator::evaluateRecursive(std::shared_ptr<ASTNode> node) {
    if (!node) return false;
    
    switch (node->type) {
        case NodeType::VARIABLE: {
            auto it = assignment.find(node->value);
            if (it != assignment.end()) {
                return it->second;
            } else {
                throw std::runtime_error("Variável não encontrada: " + node->value);
            }
        }
        case NodeType::NOT:
            return !evaluateRecursive(node->left);
        case NodeType::AND:
            return evaluateRecursive(node->left) && evaluateRecursive(node->right);
        case NodeType::OR:
            return evaluateRecursive(node->left) || evaluateRecursive(node->right);
        case NodeType::IMPLIES:
            return !evaluateRecursive(node->left) || evaluateRecursive(node->right);
        default:
            throw std::runtime_error("Tipo de nó desconhecido");
    }
}

bool Evaluator::isTautology(std::shared_ptr<ASTNode> root, const std::vector<std::string>& variables) {
    resetMetrics();
    
    auto truthTable = FileUtils::generateTruthTable(variables.size());
    
    for (const auto& row : truthTable) {
        std::unordered_map<std::string, bool> currentAssignment;
        for (size_t i = 0; i < variables.size(); ++i) {
            currentAssignment[variables[i]] = row[i];
        }
        
        setAssignment(currentAssignment);
        
        if (!evaluate(root)) {
            return false; 
        }
    }
    
    return true;
}

void Evaluator::resetMetrics() {
    metrics = Metrics();
}