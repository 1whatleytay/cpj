#pragma once

#include <parser/kinds.h>

struct ForNode : public Node {
    explicit ForNode(Node *parent);
};
