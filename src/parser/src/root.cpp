#include <parser/root.h>

#include <parser/action.h>
#include <parser/expression.h>

RootNode::RootNode(State &state) : Node(state, Kinds::Root) {
    // comments
//    spaceStoppable = [this](const char *text, size_t size) {
//        if (*text == '#') {
//            this->state.pop(
//                this->state.until([](const char *text, size_t) {
//                    return *text == '\n';
//                }),
//                notSpace);
//
//            return true;
//        }
//
//        return notSpace(text, size);
//    };

    while (state.index != state.text.size()) { // no end of file indication yettt
        push<ActionNode>();
    }
}
