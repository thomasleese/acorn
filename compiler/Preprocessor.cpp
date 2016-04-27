//
// Created by Thomas Leese on 25/04/2016.
//

#include <iostream>

#include "Errors.h"
#include "SymbolTable.h"
#include "Types.h"

#include "Preprocessor.h"
#include "PrettyPrinter.h"

using namespace jet::preprocessor;

Action::Action(Action::Kind kind, AST::Statement *statement) :
        kind(kind), statement(statement) {

}

GenericsPass::GenericsPass(SymbolTable::Namespace *root_namespace) :
        m_collecting(true) {
    m_scope.push_back(root_namespace);
}

void GenericsPass::visit(AST::CodeBlock *block) {
    int size = block->statements.size();

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

void GenericsPass::visit(AST::Identifier *identifier) {
    if (m_collecting) {
        if (identifier == m_skip_identifier) {
            return;
        }

        if (identifier->has_parameters()) {
            SymbolTable::Symbol *symbol = m_scope.back()->lookup(identifier);

            if (symbol->is_function()) {
                std::vector<SymbolTable::Symbol *> methods = symbol->nameSpace->symbols();
                for (auto sym : methods) {
                    auto def = dynamic_cast<AST::Definition *>(sym->node);
                    assert(def);
                    m_generics[def].push_back(identifier->parameters);
                }
            } else {
                if (!symbol->is_builtin) {
                    auto def = dynamic_cast<AST::Definition *>(symbol->node);
                    assert(def);
                    m_generics[def].push_back(identifier->parameters);
                }
            }
        }
    } else {
        for (auto p : identifier->parameters) {
            p->accept(this);
        }

        SymbolTable::Symbol *symbol = m_scope.back()->lookup(identifier);
        auto it = m_replacements.find(symbol);
        if (it != m_replacements.end()) {
            std::cout << "replacing " << identifier->value << " with " << it->second << std::endl;
            identifier->value = it->second;
        }

        if (identifier->has_parameters()) {
            if (symbol->is_function()) {
                identifier->parameters.clear();
            } else {
                //identifier->collapse_parameters();
                std::cout << identifier->value << std::endl;
            }
        }
    }
}

void GenericsPass::visit(AST::BooleanLiteral *boolean) {

}

void GenericsPass::visit(AST::IntegerLiteral *expression) {

}

void GenericsPass::visit(AST::FloatLiteral *expression) {

}

void GenericsPass::visit(AST::ImaginaryLiteral *imaginary) {

}

void GenericsPass::visit(AST::StringLiteral *expression) {

}

void GenericsPass::visit(AST::SequenceLiteral *sequence) {

}

void GenericsPass::visit(AST::MappingLiteral *mapping) {

}

void GenericsPass::visit(AST::Argument *argument) {

}

void GenericsPass::visit(AST::Call *expression) {
    expression->operand->accept(this);
}

void GenericsPass::visit(AST::CCall *expression) {
    for (auto p : expression->parameters) {
        p->accept(this);
    }

    for (auto a : expression->arguments) {
        a->accept(this);
    }

    expression->returnType->accept(this);
}

void GenericsPass::visit(AST::Assignment *expression) {

}

void GenericsPass::visit(AST::Selector *expression) {

}

void GenericsPass::visit(AST::Index *expression) {

}

void GenericsPass::visit(AST::Comma *expression) {

}

void GenericsPass::visit(AST::While *expression) {

}

void GenericsPass::visit(AST::For *expression) {

}

void GenericsPass::visit(AST::If *expression) {

}

void GenericsPass::visit(AST::Return *expression) {
    expression->expression->accept(this);
}

void GenericsPass::visit(AST::Spawn *expression) {

}

void GenericsPass::visit(AST::Parameter *parameter) {
    parameter->name->accept(this);
    parameter->typeNode->accept(this);
    if (parameter->defaultExpression) {
        parameter->defaultExpression->accept(this);
    }
}

void GenericsPass::visit(AST::VariableDefinition *definition) {
    definition->name->accept(this);

    if (definition->typeNode) {
        definition->typeNode->accept(this);
    }

    definition->expression->accept(this);
}

void GenericsPass::visit(AST::FunctionDefinition *definition) {
    SymbolTable::Symbol *functionSymbol = m_scope.back()->lookup(definition->name);
    SymbolTable::Symbol *symbol = functionSymbol->nameSpace->lookup_by_node(definition);
    m_scope.push_back(symbol->nameSpace);

    definition->name->accept(this);

    for (auto p : definition->parameters) {
        p->accept(this);
    }

    definition->code->accept(this);

    definition->returnType->accept(this);

    m_scope.pop_back();
}

void GenericsPass::visit(AST::TypeDefinition *definition) {
    SymbolTable::Symbol *symbol = m_scope.back()->lookup_by_node(definition);
    m_scope.push_back(symbol->nameSpace);

    definition->name->accept(this);

    if (definition->alias) {
        definition->alias->accept(this);
    } else {
        for (auto field : definition->fields) {
            field->accept(this);
        }
    }

    m_scope.pop_back();
}

void GenericsPass::visit(AST::DefinitionStatement *statement) {
    if (m_collecting) {
        if (statement->definition->name->has_parameters()) {
            m_generics[statement->definition] = std::vector<std::vector<AST::Identifier *> >();
        }

        // to avoid collecting this as a 'usage'
        m_skip_identifier = statement->definition->name;

        statement->definition->accept(this);

        m_skip_identifier = nullptr;
    } else {
        if (statement->definition->name->has_parameters()) {
            if (m_replacements.empty()) {
                auto it = m_generics.find(statement->definition);
                if (it != m_generics.end()) {
                    auto p = it->second;

                    m_actions.push_back(Action(Action::DropStatement));

                    SymbolTable::Symbol *symbol = m_scope.back()->lookup(statement->definition->name);
                    SymbolTable::Namespace *symbol_scope = m_scope.back();

                    bool was_function = symbol->is_function();

                    if (symbol->is_function()) {
                        symbol_scope = symbol->nameSpace;
                        symbol = symbol_scope->lookup_by_node(statement->definition);
                    }

                    for (auto parameters : p) {
                        if (statement->definition->name->parameters.size() == parameters.size()) {
                            auto new_statement = statement->clone();

                            /*auto builder = new SymbolTable::Builder();
                            new_statement->accept(builder);

                            auto new_symbol = builder->rootNamespace()->lookup_by_node(new_statement);*/

                            auto new_symbol = symbol->clone();
                            new_symbol->name = symbol->name + "_";
                            symbol_scope->insert(new_statement->definition, new_symbol);

                            m_replacements.clear();
                            for (int i = 0; i < parameters.size(); i++) {
                                SymbolTable::Symbol *s = new_symbol->nameSpace->lookup(statement->definition->name->parameters[i]);
                                m_replacements[s] = parameters[i]->value;
                                std::cout << s->name << "=" << parameters[i]->value << std::endl;
                            }

                            assert(!m_replacements.empty());

                            new_statement->accept(this);
                            m_replacements.clear();

                            if (was_function) {
                                auto pointer_location = reinterpret_cast<std::uintptr_t>(new_statement->definition);
                                std::stringstream ss;
                                ss << pointer_location;

                                symbol_scope->rename(new_symbol, ss.str());
                            } else {
                                symbol_scope->rename(new_symbol, new_statement->definition->name->value);
                            }

                            m_actions.push_back(Action(Action::InsertStatement, new_statement));
                        }
                    }
                } else {
                    throw Errors::InternalError(statement, "Not identified as generic.");
                }
            } else {
                statement->definition->accept(this);
            }
        } else {
            statement->definition->accept(this);
        }
    }
}

void GenericsPass::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void GenericsPass::visit(AST::ImportStatement *statement) {

}

void GenericsPass::visit(AST::SourceFile *module) {
    m_collecting = true;
    module->code->accept(this);

    m_collecting = false;
    module->code->accept(this);
}
