add_subdirectory(src)
if ("${mark3_has_bsp}" STREQUAL "true")
    add_subdirectory(example)
endif()
