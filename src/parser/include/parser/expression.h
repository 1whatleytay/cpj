#pragma once

#include <parser/kinds.h>

struct TemplateExpressionContext : public Context {
    explicit TemplateExpressionContext(Context *parent);
};

struct ParseExpressionContext : public Context {
    explicit ParseExpressionContext(Context *parent);
};

struct LiteralExpressionContext : public Context {
    explicit LiteralExpressionContext(Context *parent);
};

struct ExpressionContext : public Context {
    explicit ExpressionContext(Context *parent);
};
