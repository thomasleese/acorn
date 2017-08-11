#include <set>

#include "keywords.h"

using std::set, std::string;

static set<string> keywords = {
    "let", "def", "type", "as", "while", "for", "in", "if", "else", "not",
    "and", "or", "end", "continue", "break", "try", "except", "raise",
    "finally", "from", "import", "return", "with", "yield", "async", "await",
    "repeat", "unless", "mutable", "spawn", "ccall", "using", "new", "inout",
    "protocol", "enum", "switch", "case", "default", "module", "builtin",
    "class", "interface", "static", "public", "private", "protected", "goto",
    "global", "virtual", "pass", "assert", "del"
};

bool acorn::parser::is_keyword(const string &name) {
    return keywords.find(name) != keywords.end();
}
