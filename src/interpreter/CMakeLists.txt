add_library(interpreter STATIC
    include/interpreter/language.h
    include/interpreter/interpreter.h
    include/interpreter/builtin.h

    src/language.cpp
    src/interpreter.cpp
    src/builtin.cpp)

target_include_directories(interpreter PUBLIC include)
target_link_libraries(interpreter PUBLIC parser fmt)
