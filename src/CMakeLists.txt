find_package(Filesystem REQUIRED)

set(src
    gaf/main.cc
    gaf/args.h gaf/args.cc
    
    gaf/types.h gaf/types.cc
    gaf/parse.cc gaf/parse.h
    gaf/generator.cc gaf/generator.h

    gaf/gen_cpp.cc gaf/gen_cpp.h
    gaf/gen_imgui.cc gaf/gen_imgui.h
    gaf/gen_rapidjson.cc gaf/gen_rapidjson.h
    gaf/gen_pugixml.cc gaf/gen_pugixml.h
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


set(lib
    gaf/lib_pugixml.h gaf/lib_pugixml.cc
)

add_library(libgaf_xml STATIC ${lib})
target_include_directories(libgaf_xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libgaf_xml
    PUBLIC
        pugixml
        fmt
    PRIVATE
        project_options
        project_warnings
)
