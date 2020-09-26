#include <parser/if.h>

#include <parser/is.h>
#include <parser/for.h>
#include <parser/variable.h>
#include <parser/expression.h>

IfNode::IfNode(Node *parent) : Node(parent, Kinds::If) {
    match("if", true);

    push<LiteralExpressionNode>();

    needs("{");

    while (!peek("}")) {
        push<
            IsNode,
            IfNode,
            ForNode,
            ParseExpressionNode,
            VariableNode,
            ExpressionNode
        >();
    }

    needs("}");
}
