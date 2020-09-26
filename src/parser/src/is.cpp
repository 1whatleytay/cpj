#include <parser/is.h>

#include <parser/expression.h>

IsNode::IsNode(Node *parent) : Node(parent, Kinds::Is) {
    match("is", true);

    push<ExpressionNode>();
}
