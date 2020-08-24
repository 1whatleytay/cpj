#pragma once

#include <parser/kinds.h>

struct ForContext : public Context {
    explicit ForContext(Context *parser);
};
