project(log_example)

set(BIN_SOURCES
    main.cpp
)

mark3_add_executable(log_example ${BIN_SOURCES})

target_link_libraries(log_example.elf
    mark3
    logger
    memutil
)
