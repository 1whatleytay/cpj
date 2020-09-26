#include <parser/reference.h>

ReferenceNode::ReferenceNode(Node *parent) : Node(parent, Kinds::Reference) {
    level = MatchLevel::Light;
    name = token();
}
