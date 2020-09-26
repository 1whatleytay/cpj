#pragma once

#include <parser/kinds.h>

struct ActionNode : public Node {
    std::string name;
    size_t params = 0;
    bool lastIsVariable = false;

    explicit ActionNode(Node *parent);
};
