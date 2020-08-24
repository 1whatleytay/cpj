#include <parser/reference.h>

ReferenceContext::ReferenceContext(Context *parent) : Context(parent, KindReference) {
    name = token();
}
