#pragma once

#include <parser/kinds.h>

struct IsContext : public Context {
    explicit IsContext(Context *parent);
};
