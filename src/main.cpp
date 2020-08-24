#include <interpreter/interpreter.h>

int main(int count, const char **args) {
    interpreter::run(count, args);

    return 0;
}
