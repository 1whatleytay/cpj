add_library(parser STATIC
    include/parser/kinds.h
    include/parser/is.h
    include/parser/root.h
    include/parser/action.h
    include/parser/string.h
    include/parser/variable.h
    include/parser/reference.h
    include/parser/expression.h
    include/parser/if.h
    include/parser/for.h

    src/is.cpp
    src/root.cpp
    src/action.cpp
    src/string.cpp
    src/variable.cpp
    src/reference.cpp
    src/expression.cpp
    src/if.cpp
    src/for.cpp)

target_include_directories(parser PUBLIC include)
target_link_libraries(parser PUBLIC hermes)
