#include <parser/for.h>

#include <parser/is.h>
#include <parser/if.h>
#include <parser/variable.h>
#include <parser/expression.h>

ForContext::ForContext(Context *parser) : Context(parser, KindFor) {
    needs("for", true);

    push<LiteralExpressionContext>();

    needs("{");

    while (!next("}")) {
        push({
            link<IsContext>(),
            link<IfContext>(),
            link<ForContext>(),
            link<VariableContext>(),
            link<ExpressionContext>()
        });
    }
}
