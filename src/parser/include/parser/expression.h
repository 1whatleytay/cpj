#pragma once

#include <parser/kinds.h>

struct TemplateExpressionNode : public Node {
    explicit TemplateExpressionNode(Node *parent);
};

struct ParseExpressionNode : public Node {
    explicit ParseExpressionNode(Node *parent);
};

struct ReplaceExpressionNode : public Node {
    explicit ReplaceExpressionNode(Node *parent);
};

struct LiteralExpressionNode : public Node {
    explicit LiteralExpressionNode(Node *parent);
};

struct ExpressionNode : public Node {
    explicit ExpressionNode(Node *parent);
};
