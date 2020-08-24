#include <parser/variable.h>

#include <parser/expression.h>

VariableContext::VariableContext(Context *parent) : Context(parent, KindVariable) {
    name = token();

    needs("=");
    mark(MatchLevel::Strong);

    push<ExpressionContext>();
}

VariableContext::VariableContext(Context *parent, std::string name)
    : Context(parent, KindVariable), name(std::move(name)) { }
