#include <parser/variable.h>

#include <parser/expression.h>

VariableNode::VariableNode(Node *parent) : Node(parent, Kinds::Variable) {
    name = token();

    needs("=");
    level = MatchLevel::Strong;

    push<ExpressionNode>();
}

VariableNode::VariableNode(Node *parent, std::string name)
    : Node(parent, Kinds::Variable), name(std::move(name)) { }
