#pragma once

#include <parser/kinds.h>

struct IfContext : public Context {
    explicit IfContext(Context *parent);
};
