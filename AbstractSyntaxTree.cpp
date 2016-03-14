//
// Created by Thomas Leese on 13/03/2016.
//

#include <sstream>

#include "Lexer.h"
#include "AbstractSyntaxTree.h"

using namespace AST;

Node::~Node() {

}

std::string CodeBlock::pprint() const {
    std::stringstream ss;
    ss << "(CodeBlock " << "\n";

    for (auto statement : this->statements) {
        ss << "  " << statement->pprint() << "\n";
    }

    ss << ")";

    return ss.str();
}

std::string TypeDeclaration::pprint() const {
    return "(TypeDeclaration " + name->pprint() + ")";
}

Module::Module(std::string name) {
    this->name = name;
    this->code = new CodeBlock();
}

std::string Module::pprint() const {
    return "(Module " + name + ")";
}

std::string Parameter::pprint() const {
    return "(Parameter " + type->pprint() + " " + name->pprint() + ")";
}

std::string Identifier::pprint() const {
    return "(Identifier " + name + ")";
}

std::string IntegerLiteral::pprint() const {
    std::stringstream ss;
    ss << "(IntegerLiteral " << value << ")";
    return ss.str();
}

std::string StringLiteral::pprint() const {
    return "(StringLiteral " + value + ")";
}

std::string FunctionCall::pprint() const {
    std::stringstream ss;

    ss << "(FunctionCall " << operand->pprint();

    for (auto it = arguments.begin(); it != arguments.end(); it++) {
        ss << "  " << it->first->pprint() << " " << it->second->pprint();
    }

    ss << ")";

    return ss.str();
}

std::string Selector::pprint() const {
    return "(Selector " + operand->pprint() + " " + name->pprint() + ")";
}

std::string LetStatement::pprint() const {
    return "(LetStatement " + name->pprint() + " " + type->pprint() + " " + expression->pprint() + ")";
}

std::string DefStatement::pprint() const {
    return "(DefStatement " + name->pprint() + " " + type->pprint() + ")";
}

std::string TypeStatement::pprint() const {
    return "(TypeStatement " + name->pprint() + ")";
}

std::string ExpressionStatement::pprint() const {
    return "(ExpressionStatement " + expression->pprint() + ")";
}
