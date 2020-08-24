#include <parser/root.h>

#include <parser/action.h>
#include <parser/expression.h>

RootContext::RootContext(State &state) : Context(state, KindRoot) {
    while (state.index != state.text.size()) { // no end of file indication yettt
        push<ActionContext>();
    }
}
