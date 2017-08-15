---
layout: page
title: Compiler
permalink: /compiler/
---

## AST

```python
class Node:
    token: Token
    kind: call, def, class, etc
    type: Type

    accept(vistor: Visitor)
    clone()
    replace(a: Node, b: Node)
    children(): [Node]

class Block(Node):

class Def(Node):

class DefHolder(Node):

```

```python
class Visitor:

class Transformer(Visitor):
    def visit(): return None, self, or other
```

## Stages

- Lexer -> Tokens
- Parser -> AST, Symbol Table
- Intermediate Representation (Macro expansion) -> Lowered AST
- Type Checker -> Typed AST
- Code Generator -> LLVM IR
