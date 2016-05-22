//
// Created by Thomas Leese on 22/05/2016.
//

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>

#include "../ast/nodes.h"
#include "../errors.h"
#include "../symboltable.h"
#include "types.h"

#include "checker.h"

using namespace acorn;
using namespace acorn::typing;

#define return_if_null(thing) if (thing == nullptr) return;
#define return_if_null_type(node) return_if_null(node->type)

Checker::Checker(symboltable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Checker::~Checker() {

}

void Checker::check_types(ast::Node *lhs, ast::Node *rhs) {
    check_not_null(lhs);
    check_not_null(rhs);

    bool compatible = lhs->type->isCompatible(rhs->type);
    if (!compatible) {
        push_error(new errors::TypeMismatchError(rhs, lhs));
    }
}

void Checker::check_not_null(ast::Node *node) {
    if (!node->type) {
        push_error(new errors::InternalError(node, "No type given for: " + Token::rule_string(node->token->rule)));
    }
}

void Checker::visit(ast::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }

    check_not_null(block);
}

void Checker::visit(ast::Identifier *identifier) {
    for (auto p : identifier->parameters) {
        p->accept(this);
    }

    check_not_null(identifier);
}

void Checker::visit(ast::VariableDeclaration *node) {
    if (node->has_given_type()) {
        node->given_type()->accept(this);
    }

    check_not_null(node);
}

void Checker::visit(ast::IntegerLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(ast::FloatLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(ast::ImaginaryLiteral *imaginary) {
    check_not_null(imaginary);
}

void Checker::visit(ast::StringLiteral *expression) {
    check_not_null(expression);
}

void Checker::visit(ast::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    check_not_null(sequence);
}

void Checker::visit(ast::MappingLiteral *mapping) {
    for (int i = 0; i < mapping->keys.size(); i++) {
        mapping->keys[i]->accept(this);
        mapping->values[i]->accept(this);
    }

    check_not_null(mapping);
}

void Checker::visit(ast::RecordLiteral *expression) {
    expression->name->accept(this);

    for (auto value : expression->field_values) {
        value->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(ast::TupleLiteral *expression) {
    for (auto element : expression->elements()) {
        element->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(ast::Call *expression) {
    expression->operand->accept(this);

    for (auto arg : expression->arguments) {
        arg->accept(this);
    }

    check_not_null(expression);
}

void Checker::visit(ast::CCall *ccall) {
    // ccall->name->accept(this);

    for (auto param : ccall->parameters) {
        param->accept(this);
    }

    ccall->returnType->accept(this);

    for (auto arg : ccall->arguments) {
        arg->accept(this);
    }

    check_not_null(ccall);
}

void Checker::visit(ast::Cast *cast) {
    cast->operand->accept(this);
    cast->new_type->accept(this);

    check_not_null(cast);
}

void Checker::visit(ast::Assignment *expression) {
    expression->lhs->accept(this);
    expression->rhs->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Selector *expression) {
    expression->operand->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::While *expression) {
    expression->condition()->accept(this);
    expression->code()->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::If *expression) {
    expression->condition->accept(this);
    expression->trueCode->accept(this);
    if (expression->falseCode) {
        expression->falseCode->accept(this);
    }
    check_not_null(expression);
}

void Checker::visit(ast::Return *expression) {
    expression->expression->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Spawn *expression) {
    expression->call->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Sizeof *expression) {
    expression->identifier->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Strideof *expression) {
    expression->identifier->accept(this);
    check_not_null(expression);
}

void Checker::visit(ast::Parameter *parameter) {
    parameter->typeNode->accept(this);

    check_not_null(parameter);
}

void Checker::visit(ast::VariableDefinition *definition) {
    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

    definition->assignment->accept(this);

    check_types(definition, definition->assignment->rhs);
}

void Checker::visit(ast::FunctionDefinition *definition) {
    check_not_null(definition);

    auto functionSymbol = m_namespace->lookup(this, definition->name);

    types::Method *method = static_cast<types::Method *>(definition->type);

    auto symbol = functionSymbol->nameSpace->lookup(this, definition, method->mangled_name());

    symboltable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    // it's valid for the name not to have a type, since it's doesn't exist
    // definition->name->accept(this);

    for (auto p : definition->name->parameters) {
        p->accept(this);
    }

    definition->returnType->accept(this);

    for (auto p : definition->parameters) {
        p->accept(this);
    }

    definition->code->accept(this);

    m_namespace = oldNamespace;
}

void Checker::visit(ast::TypeDefinition *definition) {
    // it's valid for the name not to have a type, since it's doesn't exist
    //definition->name->accept(this);

    auto symbol = m_namespace->lookup(this, definition->name);

    symboltable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        /*for (auto name : definition->field_names) {
            name->accept(this);
        }*/

        for (auto type : definition->field_types) {
            type->accept(this);
        }
    }

    check_not_null(definition);

    m_namespace = oldNamespace;
}

void Checker::visit(ast::DefinitionStatement *statement) {
    statement->definition->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::ExpressionStatement *statement) {
    statement->expression->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::ImportStatement *statement) {
    statement->path->accept(this);
    check_not_null(statement);
}

void Checker::visit(ast::SourceFile *module) {
    module->code->accept(this);
    check_not_null(module);
}
