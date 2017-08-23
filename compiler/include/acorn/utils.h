//
// Created by Thomas Leese on 14/08/2017.
//

#pragma once

#define return_if_null(thing) \
    if (thing == nullptr) { return; }

#define return_if_false(thing) \
    if (thing == false) { return; }

#define return_if_null_type(node) \
    return_if_false(node->has_type())

#define return_and_push_null_if_null(thing) \
    if (thing == nullptr) { push_llvm_value(nullptr); return; }

#define return_null_and_push_null_if_null(thing) \
    if (thing == nullptr) { push_llvm_value(nullptr); return nullptr; }

#define return_null_if_null(thing) \
    if (thing == nullptr) { return nullptr; }

#define return_node_if_null(thing) \
    if (thing == nullptr) { return node; }

#define return_null_if_false(thing) \
    if (thing == false) { return nullptr; }

#define return_null_if_has_errors(thing) \
    if ((thing).has_errors()) { return nullptr; }
