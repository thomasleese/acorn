//
// Created by Thomas Leese on 09/04/2017.
//

#include <cassert>
#include <iostream>
#include <sstream>

#include "generics.h"
#include "symboltable.h"

using namespace acorn;
using namespace acorn::generics;

Reification::Reification(symboltable::Namespace *scope) {
    push_scope(scope);
}

void Reification::visit(ast::Block *node) {

}

void Reification::visit(ast::Name *node) {

}

void Reification::visit(ast::VariableDeclaration *node) {

}

void Reification::visit(ast::Int *node) {

}

void Reification::visit(ast::Float *node) {

}

void Reification::visit(ast::Complex *node) {

}

void Reification::visit(ast::String *node) {

}

void Reification::visit(ast::List *node) {

}

void Reification::visit(ast::Dictionary *node) {

}

void Reification::visit(ast::Tuple *node) {

}

void Reification::visit(ast::Call *node) {

}

void Reification::visit(ast::CCall *node) {

}

void Reification::visit(ast::Cast *node) {

}

void Reification::visit(ast::Assignment *node) {

}

void Reification::visit(ast::Selector *node) {

}

void Reification::visit(ast::While *node) {

}

void Reification::visit(ast::If *node) {

}

void Reification::visit(ast::Return *node) {

}

void Reification::visit(ast::Spawn *node) {

}

void Reification::visit(ast::Switch *node) {

}

void Reification::visit(ast::Parameter *node) {

}

void Reification::visit(ast::Let *node) {

}

void Reification::visit(ast::Def *node) {

}

void Reification::visit(ast::Type *node) {

}

void Reification::visit(ast::Module *node) {

}

void Reification::visit(ast::Import *node) {

}

void Reification::visit(ast::SourceFile *node) {

}
