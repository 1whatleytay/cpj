#pragma once

#include <parser/kinds.h>

struct ReferenceContext : public Context {
    std::string name;

    explicit ReferenceContext(Context *parent);
};
