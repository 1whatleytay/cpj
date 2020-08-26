#include <parser/if.h>

#include <parser/is.h>
#include <parser/for.h>
#include <parser/variable.h>
#include <parser/expression.h>

IfContext::IfContext(Context *parent) : Context(parent, KindIf) {
    needs("if", true);

    push<LiteralExpressionContext>();

    needs("{");

    while (!next("}")) {
        push({
            link<IsContext>(),
            link<IfContext>(),
            link<ForContext>(),
            link<ParseExpressionContext>(),
            link<VariableContext>(),
            link<ExpressionContext>()
        });
    }
}
