#pragma once

#include <parser/kinds.h>

struct VariableNode : public Node {
    std::string name;

    explicit VariableNode(Node *parent);
    explicit VariableNode(Node *parent, std::string name);
};
