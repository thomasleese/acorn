//
// Created by Thomas Leese on 21/03/2016.
//

#include <iostream>
#include <set>

#include "Typing.h"
#include "Errors.h"

#include "SymbolTable.h"

using namespace Typing;

Inferrer::Inferrer(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Inferrer::~Inferrer() {

}

Types::Constructor *Inferrer::find_type_constructor(AST::Node *node, std::string name) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(node, name);
    Types::Constructor *typeConstructor = dynamic_cast<Types::Constructor *>(symbol->type);
    if (typeConstructor) {
        return typeConstructor;
    } else {
        return nullptr;
    }
}

Types::Parameter *Inferrer::find_type_parameter(AST::Node *node, std::string name) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(node, name);
    Types::Parameter *typeParameter = dynamic_cast<Types::Parameter *>(symbol->type);
    if (typeParameter) {
        return typeParameter;
    } else {
        return nullptr;
    }
}

Types::Type *Inferrer::find_type(AST::Node *node, std::string name, std::vector<AST::Type *> parameters) {
    std::vector<Types::Type *> parameterTypes;

    for (auto parameter : parameters) {
        parameter->accept(this);
        parameterTypes.push_back(parameter->type);
    }

    Types::Constructor *typeConstructor = find_type_constructor(node, name);
    if (typeConstructor != nullptr) {
        return typeConstructor->create(node, parameterTypes);
    } else {
        Types::Parameter *typeParameter = find_type_parameter(node, name);
        if (typeParameter != nullptr) {
            return typeParameter;
        } else {
            throw Errors::InvalidTypeConstructor(node);
        }
    }
}

Types::Type *Inferrer::find_type(AST::Node *node, std::string name) {
    return find_type(node, name, std::vector<AST::Type *>());
}

Types::Type *Inferrer::find_type(AST::Type *type) {
    return find_type(type, type->name->name, type->parameters);
}

void Inferrer::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Inferrer::visit(AST::Identifier *expression) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(expression, expression->name);
    expression->type = symbol->type;
}

void Inferrer::visit(AST::BooleanLiteral *expression) {
    expression->type = find_type(expression, "Boolean");
}

void Inferrer::visit(AST::IntegerLiteral *expression) {
    expression->type = find_type(expression, "Integer");
}

void Inferrer::visit(AST::FloatLiteral *expression) {
    expression->type = find_type(expression, "Float");
}

void Inferrer::visit(AST::ImaginaryLiteral *expression) {
    expression->type = find_type(expression, "Complex");
}

void Inferrer::visit(AST::StringLiteral *expression) {
    expression->type = find_type(expression, "String");
}

void Inferrer::visit(AST::SequenceLiteral *sequence) {
    for (auto element : sequence->elements) {
        element->accept(this);
    }

    std::set<Types::Type *> types;
    for (auto element : sequence->elements) {
        bool inList = false;
        for (auto type : types) {
            if (*type == *(element->type)) {
                inList = true;
                break;
            }
        }

        if (!inList) {
            types.insert(element->type);
        }
    }

    Types::Type *elementType;
    if (types.empty()) {
        elementType = new Types::Any();
    } else if (types.size() != 1) {
        elementType = new Types::Union(sequence, types);
    } else {
        elementType = *(types.begin());
    }

    sequence->type = new Types::Sequence(elementType);
}

void Inferrer::visit(AST::MappingLiteral *mapping) {
    throw Errors::TypeInferenceError(mapping);
}

void Inferrer::visit(AST::Argument *argument) {
    throw Errors::TypeInferenceError(argument);
}

void Inferrer::visit(AST::Call *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::Assignment *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::Selector *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::While *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::For *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::If *expression) {
    throw Errors::TypeInferenceError(expression);
}

void Inferrer::visit(AST::Type *type) {
    type->type = find_type(type);
}

void Inferrer::visit(AST::Cast *cast) {
    cast->typeNode->accept(this);
    cast->type = cast->typeNode->type;
}

void Inferrer::visit(AST::Parameter *parameter) {
    throw Errors::TypeInferenceError(parameter);
}

void Inferrer::visit(AST::VariableDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name);

    definition->cast->accept(this);
    definition->expression->accept(this);

    Types::Type *type = definition->cast->type;
    symbol->type = type;
    definition->type = type;
}

void Inferrer::visit(AST::FunctionDefinition *definition) {
    throw Errors::TypeInferenceError(definition);
}

void Inferrer::visit(AST::TypeDefinition *definition) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(definition, definition->name->name->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    Types::Type *type;
    if (definition->alias) {
        std::vector<Types::Type *> inputParameters;
        for (auto t : definition->name->parameters) {
            t->accept(this);
            inputParameters.push_back(t->type);
        }

        std::vector<Types::Type *> outputParameters;
        for (auto t : definition->alias->parameters) {
            t->accept(this);
            outputParameters.push_back(t->type);
        }

        type = new Types::AliasConstructor(definition, find_type_constructor(definition, definition->alias->name->name),
                                           inputParameters, outputParameters);
    } else {
        type = new Types::RecordConstructor();
    }

    symbol->type = type;
    definition->type = type;

    m_namespace = oldNamespace;
}

void Inferrer::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Inferrer::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Inferrer::visit(AST::Module *module) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(module, module->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    module->code->accept(this);

    m_namespace = oldNamespace;
}

Checker::Checker(SymbolTable::Namespace *rootNamespace) {
    m_namespace = rootNamespace;
}

Checker::~Checker() {

}

void Checker::check_types(AST::Node *lhs, AST::Node *rhs) {
    bool compatible = lhs->type->isCompatible(rhs->type);
    if (!compatible) {
        throw Errors::TypeMismatchError(lhs, rhs);
    }
}

void Checker::visit(AST::CodeBlock *block) {
    for (auto statement : block->statements) {
        statement->accept(this);
    }
}

void Checker::visit(AST::Identifier *expression) {

}

void Checker::visit(AST::BooleanLiteral *boolean) {

}

void Checker::visit(AST::IntegerLiteral *expression) {

}

void Checker::visit(AST::FloatLiteral *expression) {

}

void Checker::visit(AST::ImaginaryLiteral *imaginary) {

}

void Checker::visit(AST::StringLiteral *expression) {

}

void Checker::visit(AST::SequenceLiteral *sequence) {

}

void Checker::visit(AST::MappingLiteral *mapping) {

}

void Checker::visit(AST::Argument *argument) {

}

void Checker::visit(AST::Call *expression) {

}

void Checker::visit(AST::Assignment *expression) {

}

void Checker::visit(AST::Selector *expression) {

}

void Checker::visit(AST::While *expression) {

}

void Checker::visit(AST::For *expression) {

}

void Checker::visit(AST::If *expression) {

}

void Checker::visit(AST::Type *type) {

}

void Checker::visit(AST::Cast *cast) {

}

void Checker::visit(AST::Parameter *parameter) {

}

void Checker::visit(AST::VariableDefinition *definition) {
    check_types(definition, definition->expression);
}

void Checker::visit(AST::FunctionDefinition *definition) {

}

void Checker::visit(AST::TypeDefinition *definition) {

}

void Checker::visit(AST::DefinitionStatement *statement) {
    statement->definition->accept(this);
}

void Checker::visit(AST::ExpressionStatement *statement) {
    statement->expression->accept(this);
}

void Checker::visit(AST::Module *module) {
    SymbolTable::Symbol *symbol = m_namespace->lookup(module, module->name);

    SymbolTable::Namespace *oldNamespace = m_namespace;
    m_namespace = symbol->nameSpace;

    module->code->accept(this);

    m_namespace = oldNamespace;
}
