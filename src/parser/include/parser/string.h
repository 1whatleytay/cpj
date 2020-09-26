#pragma once

#include <parser/kinds.h>

struct StringNode : public Node {
    std::string text;

    std::vector<size_t> inserts;

    explicit StringNode(Node *parent);
};
