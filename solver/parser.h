#ifndef PARSER_H
#define PARSER_H

#include "../common/ast.h"
#include <string>
#include <memory>
#include <stack>

class Parser {
private:
    std::string formula;
    size_t pos;
    
    void skipWhitespace();
    char peek();
    char consume();
    bool isVariable(char c);
    
    bool checkOperator(const std::string& op);
    void consumeOperator(const std::string& op);
    
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseImplication();
    std::shared_ptr<ASTNode> parseOr();
    std::shared_ptr<ASTNode> parseAnd();
    std::shared_ptr<ASTNode> parseNot();
    std::shared_ptr<ASTNode> parsePrimary();
    
public:
    Parser(const std::string& formula);
    std::shared_ptr<ASTNode> parse();
};

#endif
