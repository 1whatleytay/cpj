#pragma once

#include <parser/kinds.h>

struct VariableContext : public Context {
    std::string name;

    explicit VariableContext(Context *parent);
    explicit VariableContext(Context *parent, std::string name);
};
