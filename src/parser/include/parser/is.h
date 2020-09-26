#pragma once

#include <parser/kinds.h>

struct IsNode : public Node {
    explicit IsNode(Node *parent);
};
