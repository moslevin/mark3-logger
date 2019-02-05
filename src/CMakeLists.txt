project(logger)

set(LIB_SOURCES
    logbuf.cpp
)

set(LIB_HEADERS
    public/fnv_hash32.h
    public/logbuf.h
    public/logmacro.h
    public/logtypes.h
)

mark3_add_library(logger ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(logger
    PUBLIC
        public
    )

target_link_libraries(logger
    mark3
    )
