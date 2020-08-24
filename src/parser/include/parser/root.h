#pragma once

#include <parser/kinds.h>

struct RootContext : public Context {
    explicit RootContext(State &state);
};
