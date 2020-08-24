add_library({{ name }} STATIC
    {{ includes }}

    {{ sources }})

target_include_directories({{ name }} PUBLIC include)
target_link_libraries({{ name }} PUBLIC {{ links }})
