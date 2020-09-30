#include <parser/action.h>

#include <parser/is.h>
#include <parser/if.h>
#include <parser/for.h>
#include <parser/variable.h>
#include <parser/expression.h>

ActionNode::ActionNode(Node *parent) : Node(parent, Kinds::Action) {
    name = token();

    while (!peek("{")) {
        params++;
        push<VariableNode>(false, token());

        if (next("...")) {
            lastIsVariable = true;
            break;
        }
    }

    match("{");

    while (!peek("}")) {
        push<IsNode, IfNode, ForNode, ParseExpressionNode, VariableNode, ExpressionNode>();
    }

    needs("}");
}
