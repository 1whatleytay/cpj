#pragma once

#include <parser/kinds.h>

struct ActionContext : public Context {
    std::string name;
    size_t params = 0;
    bool lastIsVariable = false;

    explicit ActionContext(Context *parent);
};
