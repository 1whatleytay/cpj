#pragma once

#include <parser/kinds.h>

struct StringContext : public Context {
    std::string text;

    std::vector<size_t> inserts;

    explicit StringContext(Context *parent);
};
