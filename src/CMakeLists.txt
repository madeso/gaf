set(src
    gaf/main.cc
)
source_group("" FILES ${src})

add_executable(gaf ${src})
target_include_directories(gaf
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
