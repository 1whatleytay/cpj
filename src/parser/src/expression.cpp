#include <parser/expression.h>

#include <parser/string.h>
#include <parser/reference.h>

bool onNewline(const char *text, size_t) {
    return *text == '\n' || !std::isspace(*text);
};

TemplateExpressionContext::TemplateExpressionContext(Context *parent) : Context(parent, KindTemplateExpression) {
    popStoppable = notSpace;

    push<LiteralExpressionContext>();

    needs("{");
    level = MatchLevel::Strong;

    while (!next("}")) {
        push<StringContext>();

        needs(":");

        push<LiteralExpressionContext>();
    }
}

LiteralExpressionContext::LiteralExpressionContext(Context *parent) : Context(parent, KindLiteralExpression) {
    if (next("(")) {
        push<ExpressionContext>();
        needs(")");
    } else {
        push({
            link<StringContext>(),
            link<ReferenceContext>()
        });
    }
}

ExpressionContext::ExpressionContext(Context *parent) : Context(parent, KindExpression) {
    if (push({ link<TemplateExpressionContext>(), link<StringContext>() }, true))
        return;

    popStoppable = onNewline;

    push<ReferenceContext>();

    popStoppable = notSpace;
    while (!next("\n") && !peek(")") && !peek("}")) {
        popStoppable = onNewline;
        push<LiteralExpressionContext>();
        popStoppable = notSpace;
    }

    return;
}
