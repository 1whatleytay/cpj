#include <parser/expression.h>

#include <parser/string.h>
#include <parser/reference.h>

bool onNewline(const char *text, size_t) {
    return *text == '\n' || !std::isspace(*text);
}

TemplateExpressionNode::TemplateExpressionNode(Node *parent) : Node(parent, Kinds::TemplateExpression) {
    spaceStoppable = notSpace;

    push<LiteralExpressionNode>();

    match("{");

    while (!peek("}")) {
        push<StringNode>();

        needs(":");

        push<LiteralExpressionNode>();
    }

    needs("}");
}

ParseExpressionNode::ParseExpressionNode(Node *parent) : Node(parent, Kinds::ParseExpression) {
    push<LiteralExpressionNode>();

    match("->");

    spaceStoppable = notSpace;

    push<LiteralExpressionNode>();

    needs("{");

    while (!peek("}")) {
        push<StringNode>();

        needs(":");

        push<ReferenceNode>();
    }

    needs("}");
}

ReplaceExpressionNode::ReplaceExpressionNode(Node *parent) : Node(parent, Kinds::ReplaceExpression) {
    push<LiteralExpressionNode>();

    match("=>");

    spaceStoppable = notSpace;

    push<LiteralExpressionNode>();

    needs("{");

    while (!peek("}")) {
        push<StringNode>();

        needs(":");

        push<ReferenceNode>();

        needs("as", true);

        push<LiteralExpressionNode>();
    }

    needs("}");
}

LiteralExpressionNode::LiteralExpressionNode(Node *parent) : Node(parent, Kinds::LiteralExpression) {
    if (next("(")) {
        push<ExpressionNode>();
        needs(")");
    } else {
        push<StringNode, ReferenceNode>();
    }
}

ExpressionNode::ExpressionNode(Node *parent) : Node(parent, Kinds::Expression) {
    if (push<TemplateExpressionNode, ReplaceExpressionNode, StringNode>(true))
        return;

    spaceStoppable = onNewline;

    push<ReferenceNode>();

    spaceStoppable = notSpace;
    while (!next("\n") && !peek(")") && !peek("}")) {
        spaceStoppable = onNewline;
        push<LiteralExpressionNode>();
        spaceStoppable = notSpace;
    }
}
