#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::string& formula) : formula(formula), pos(0) {}

void Parser::skipWhitespace() {
    while (pos < formula.length() && std::isspace(formula[pos])) {
        pos++;
    }
}

char Parser::peek() {
    skipWhitespace();
    if (pos >= formula.length()) {
        return '\0';
    }
    return formula[pos];
}

char Parser::consume() {
    skipWhitespace();
    if (pos >= formula.length()) {
        return '\0';
    }
    return formula[pos++];
}

bool Parser::isVariable(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Helper function to check for Unicode operators
bool Parser::checkOperator(const std::string& op) {
    skipWhitespace();
    if (pos + op.length() > formula.length()) {
        return false;
    }
    return formula.substr(pos, op.length()) == op;
}

void Parser::consumeOperator(const std::string& op) {
    skipWhitespace();
    if (checkOperator(op)) {
        pos += op.length();
    }
}

std::shared_ptr<ASTNode> Parser::parse() {
    pos = 0;
    auto result = parseExpression();
    skipWhitespace();
    if (pos < formula.length()) {
        throw std::runtime_error("Caracteres extras no final da fórmula");
    }
    return result;
}

// Precedência: → (menor) < ∨ < ∧ < ¬ (maior)
std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseImplication();
}

std::shared_ptr<ASTNode> Parser::parseImplication() {
    auto left = parseOr();
    
    while (checkOperator("→")) {
        consumeOperator("→");
        auto right = parseOr();
        auto node = std::make_shared<ASTNode>(NodeType::IMPLIES);
        node->left = left;
        node->right = right;
        left = node;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseOr() {
    auto left = parseAnd();
    
    while (checkOperator("∨")) {
        consumeOperator("∨");
        auto right = parseAnd();
        auto node = std::make_shared<ASTNode>(NodeType::OR);
        node->left = left;
        node->right = right;
        left = node;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseAnd() {
    auto left = parseNot();
    
    while (checkOperator("∧")) {
        consumeOperator("∧");
        auto right = parseNot();
        auto node = std::make_shared<ASTNode>(NodeType::AND);
        node->left = left;
        node->right = right;
        left = node;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseNot() {
    if (checkOperator("¬")) {
        consumeOperator("¬");
        auto operand = parseNot(); // Para permitir ¬¬p
        auto node = std::make_shared<ASTNode>(NodeType::NOT);
        node->left = operand;
        return node;
    }
    
    return parsePrimary();
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    char c = peek();
    
    if (c == '(') {
        consume(); // (
        auto node = parseExpression();
        if (peek() != ')') {
            throw std::runtime_error("Esperado ')' após expressão");
        }
        consume(); // )
        return node;
    }
    
    if (isVariable(c)) {
        std::string varName(1, consume());
        return std::make_shared<ASTNode>(NodeType::VARIABLE, varName);
    }
    
    throw std::runtime_error("Token inesperado: " + std::string(1, c));
}
