add_subdirectory(parser)
add_subdirectory(interpreter)

add_executable(cpj main.cpp ../.cpj/parser.h ../.cpj/kinds.h)
target_link_libraries(cpj PRIVATE interpreter)
