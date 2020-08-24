#include <parser/action.h>

#include <parser/is.h>
#include <parser/if.h>
#include <parser/for.h>
#include <parser/variable.h>
#include <parser/expression.h>

ActionContext::ActionContext(Context *parent) : Context(parent, KindAction) {
    name = token();

    while (!next("{")) {
        params++;
        push<VariableContext>(false, token());

        if (next("...")) {
            lastIsVariable = true;
            needs("{");
            break;
        }
    }

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
