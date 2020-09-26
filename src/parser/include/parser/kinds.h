#pragma once

#include <hermes/node.h>

using namespace hermes;

enum class Kinds {
    Is,
    Root,
    Action,
    Variable,
    Expression,
    LiteralExpression,
    TemplateExpression,
    ParseExpression,
    ReplaceExpression,
    Reference,
    String,
    For,
    If,
};
