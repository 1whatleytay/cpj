#include <parser/is.h>

#include <parser/expression.h>

IsContext::IsContext(Context *parent) : Context(parent, KindIs) {
    needs("is", true);

    push<ExpressionContext>();
}
