add_subdirectory(parser)
add_subdirectory(interpreter)

add_executable(cpj main.cpp)
target_link_libraries(cpj PRIVATE interpreter)
