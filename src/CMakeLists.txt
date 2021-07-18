find_package(Filesystem REQUIRED)

set(src
    gaf/main.cc
    gaf/cpp.cc
    gaf/args.h gaf/args.cc
    # gaf/main.cc
    gaf/parse.cc
    gaf/types.h gaf/types.cc
)
source_group("" FILES ${src})

add_executable(gaf ${src})
target_include_directories(gaf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gaf
    PRIVATE
        project_options
        project_warnings
        fmt
        std::filesystem
)
