#include <parser/for.h>

#include <parser/is.h>
#include <parser/if.h>
#include <parser/variable.h>
#include <parser/expression.h>

ForNode::ForNode(Node *parent) : Node(parent, Kinds::For) {
    match("for", true);

    push<LiteralExpressionNode>();

    needs("{");

    while (!peek("}")) {
        push({
            link<IsNode>(),
            link<IfNode>(),
            link<ForNode>(),
            link<ParseExpressionNode>(),
            link<VariableNode>(),
            link<ExpressionNode>()
        });
    }

    needs("}");
}
