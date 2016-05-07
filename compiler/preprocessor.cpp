//
// Created by Thomas Leese on 25/04/2016.
//

#include <cassert>
#include <iostream>

#include "ast/nodes.h"
#include "errors.h"
#include "prettyprinter.h"
#include "symbolTable.h"
#include "types.h"

#include "preprocessor.h"

using namespace jet;
using namespace jet::preprocessor;

Action::Action(Action::Kind kind, ast::Statement *statement) :
        kind(kind), statement(statement) {

}

GenericsPass::GenericsPass(symboltable::Namespace *root_namespace) :
        m_collecting(true) {
    m_scope.push_back(root_namespace);
}

void GenericsPass::visit(ast::CodeBlock *block) {
    size_t size = block->statements.size();

    for (int i = 0; i < size; i++) {
        auto statement = block->statements[i];

        statement->accept(this);

        while (m_replacements.empty() && !m_actions.empty()) {
            auto action = m_actions.back();
            m_actions.pop_back();

            switch (action.kind) {
                case Action::DropStatement:
                    block->statements.erase(block->statements.begin() + i);
                    i--;
                    size--;
                    break;

                case Action::InsertStatement:
                    block->statements.insert(block->statements.begin() + i, action.statement);
                    i++;
                    size++;
                    break;
            }
        }
    }
}

void GenericsPass::visit(ast::Identifier *identifier) {
    if (m_collecting) {
        if (m_skip_identifier.size() && identifier == m_skip_identifier.back()) {
            return;
        }

        if (identifier->has_parameters()) {
            auto symbol = m_scope.back()->lookup(this, identifier);

            if (symbol->is_function()) {
                std::vector<symboltable::Symbol *> methods = symbol->nameSpace->symbols();
                for (auto sym : methods) {
                    auto def = dynamic_cast<ast::Definition *>(sym->node);
                    assert(def);
                    m_generics[def].push_back(identifier->parameters);
                }
            } else if (symbol->is_variable()) {
                auto def = dynamic_cast<ast::Definition *>(symbol->node);
                assert(def);
                m_generics[def].push_back(identifier->parameters);
            }
        }
    } else {
        for (auto p : identifier->parameters) {
            p->accept(this);
        }

        auto symbol = m_scope.back()->lookup(this, identifier);

        auto it = m_replacements.find(symbol);
        if (it != m_replacements.end()) {
            identifier->value = it->second;
        }

        if (identifier->has_parameters()) {
            if (symbol->is_function()) {
                identifier->parameters.clear();
            } else if (symbol->is_variable()) {
                //identifier->collapse_parameters();
            }
        }
    }
}

void GenericsPass::visit(ast::BooleanLiteral *boolean) {

}

void GenericsPass::visit(ast::IntegerLiteral *expression) {

}

void GenericsPass::visit(ast::FloatLiteral *expression) {

}

void GenericsPass::visit(ast::ImaginaryLiteral *imaginary) {

}

void GenericsPass::visit(ast::StringLiteral *expression) {

}

void GenericsPass::visit(ast::SequenceLiteral *sequence) {

}

void GenericsPass::visit(ast::MappingLiteral *mapping) {

}

void GenericsPass::visit(ast::RecordLiteral *expression) {
    expression->name->accept(this);
}

void GenericsPass::visit(ast::Argument *argument) {
    if (argument->name) {
        //argument->name->accept(this);
    }

    argument->value->accept(this);
}

void GenericsPass::visit(ast::Call *expression) {
    expression->operand->accept(this);

    for (auto arg : expression->arguments) {
        arg->accept(this);
    }
}

void GenericsPass::visit(ast::CCall *expression) {
    for (auto p : expression->parameters) {
        p->accept(this);
    }

    for (auto a : expression->arguments) {
        a->accept(this);
    }

    expression->returnType->accept(this);
}

void GenericsPass::visit(ast::Cast *expression) {
    expression->operand->accept(this);
    expression->new_type->accept(this);
}

void GenericsPass::visit(ast::Assignment *expression) {

}

void GenericsPass::visit(ast::Selector *expression) {

}

void GenericsPass::visit(ast::Index *expression) {

}

void GenericsPass::visit(ast::Comma *expression) {

}

void GenericsPass::visit(ast::While *expression) {

}

void GenericsPass::visit(ast::For *expression) {

}

void GenericsPass::visit(ast::If *expression) {
    expression->condition->accept(this);

    expression->trueCode->accept(this);

    if (expression->falseCode) {
        expression->falseCode->accept(this);
    }
}

void GenericsPass::visit(ast::Return *expression) {
    expression->expression->accept(this);
}

void GenericsPass::visit(ast::Spawn *expression) {

}

void GenericsPass::visit(ast::Sizeof *expression) {
    expression->identifier->accept(this);
}

void GenericsPass::visit(ast::Strideof *expression) {
    expression->identifier->accept(this);
}

void GenericsPass::visit(ast::Parameter *parameter) {
    parameter->name->accept(this);
    parameter->typeNode->accept(this);
    if (parameter->defaultExpression) {
        parameter->defaultExpression->accept(this);
    }
}

void GenericsPass::visit(ast::VariableDefinition *definition) {
    definition->name->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
    }

    definition->expression->accept(this);
}

void GenericsPass::visit(ast::FunctionDefinition *definition) {
    auto functionSymbol = m_scope.back()->lookup(this, definition->name);
    auto symbol = functionSymbol->nameSpace->lookup_by_node(this, definition);
    m_scope.push_back(symbol->nameSpace);

    definition->name->accept(this);

    for (auto p : definition->parameters) {
        p->accept(this);
    }

    definition->code->accept(this);

    definition->returnType->accept(this);

    m_scope.pop_back();
}

void GenericsPass::visit(ast::TypeDefinition *definition) {
    auto symbol = m_scope.back()->lookup_by_node(this, definition);
    if (symbol == nullptr) {
        return;
    }

    m_scope.push_back(symbol->nameSpace);

    definition->name->accept(this);

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        for (auto name : definition->field_names) {
            name->accept(this);
        }

        for (auto type : definition->field_types) {
            type->accept(this);
        }
    }

    m_scope.pop_back();
}

void GenericsPass::visit(ast::DefinitionStatement *statement) {
    if (!statement->definition->name->has_parameters()) {
        statement->definition->accept(this);
        return;
    }

    auto symbol = m_scope.back()->lookup(this, statement->definition->name);
    if (symbol == nullptr) {
        return;
    }

    if (m_collecting) {
        if (symbol->is_variable() || symbol->is_function()) {
            m_generics[statement->definition] = std::vector<std::vector<ast::Identifier *> >();

            // to avoid collecting this as a 'usage'
            m_skip_identifier.push_back(statement->definition->name);
            statement->definition->accept(this);
            m_skip_identifier.pop_back();
        }
    } else {
        if (symbol->is_variable() || symbol->is_function()) {
            if (m_replacements.empty()) {
                auto it = m_generics.find(statement->definition);
                if (it != m_generics.end()) {
                    auto p = it->second;

                    m_actions.push_back(Action(Action::DropStatement));

                    symboltable::Namespace *symbol_scope = m_scope.back();

                    bool was_function = symbol->is_function();

                    if (symbol->is_function()) {
                        symbol_scope = symbol->nameSpace;
                        symbol = symbol_scope->lookup_by_node(this, statement->definition);
                    }

                    for (auto parameters : p) {
                        if (statement->definition->name->parameters.size() == parameters.size()) {
                            auto new_statement = statement->clone();

                            /*auto builder = new symboltable::Builder();
                            new_statement->accept(builder);

                            auto new_symbol = builder->rootNamespace()->lookup_by_node(new_statement);*/

                            auto new_symbol = symbol->clone();
                            new_symbol->name = symbol->name + "_";
                            symbol_scope->insert(this, new_statement->definition, new_symbol);

                            m_replacements.clear();
                            for (int i = 0; i < parameters.size(); i++) {
                                auto s = new_symbol->nameSpace->lookup(this, statement->definition->name->parameters[i]);
                                m_replacements[s] = parameters[i]->value;
                            }

                            assert(!m_replacements.empty());

                            new_statement->accept(this);
                            m_replacements.clear();

                            if (was_function) {
                                auto pointer_location = reinterpret_cast<std::uintptr_t>(new_statement->definition);
                                std::stringstream ss;
                                ss << pointer_location;

                                symbol_scope->rename(this, new_symbol, ss.str());
                            } else {
                                symbol_scope->rename(this, new_symbol, new_statement->definition->name->value);
                            }

                            m_actions.push_back(Action(Action::InsertStatement, new_statement));
                        }
                    }
                } else {
                    push_error(new errors::InternalError(statement, "Not identified as generic."));
                }
            } else {
                statement->definition->accept(this);
            }
        } else {
            statement->definition->accept(this);
        }
    }
}

void GenericsPass::visit(ast::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void GenericsPass::visit(ast::ImportStatement *statement) {

}

void GenericsPass::visit(ast::SourceFile *module) {
    m_collecting = true;
    module->code->accept(this);

    m_collecting = false;
    module->code->accept(this);
}
