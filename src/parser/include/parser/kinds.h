#pragma once

#include <hermes/context.h>

using namespace hermes;

enum Kinds : size_t {
    KindIs,
    KindRoot,
    KindAction,
    KindVariable,
    KindExpression,
    KindLiteralExpression,
    KindTemplateExpression,
    KindReference,
    KindString,
    KindFor,
    KindIf,
};
