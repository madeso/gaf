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
        libgaf::project_options
        libgaf::project_warnings
        libgaf::fmt
        std::filesystem
)

set(lib_gaf
    gaf/lib_gaf.h gaf/lib_gaf.cc
)
source_group("" FILES ${lib_gaf})
add_library(libgaf_gaf STATIC ${lib_gaf})
add_library(libgaf::gaf ALIAS libgaf_gaf)
target_include_directories(libgaf_gaf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libgaf_gaf
    PRIVATE
        libgaf::project_options
        libgaf::project_warnings
)



set(lib_gaf_xml
    gaf/lib_pugixml.h gaf/lib_pugixml.cc
)
source_group("" FILES ${lib_gaf_xml})
add_library(libgaf_xml STATIC ${lib_gaf_xml})
add_library(libgaf::xml ALIAS libgaf_xml)
target_include_directories(libgaf_xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libgaf_xml
    PUBLIC
        libgaf::pugixml
        libgaf::fmt
        libgaf::gaf
    PRIVATE
        libgaf::project_options
        libgaf::project_warnings
)

set(lib_gaf_json
    gaf/lib_rapidjson.h gaf/lib_rapidjson.cc
)
source_group("" FILES ${lib_gaf_json})
add_library(libgaf_json STATIC ${lib_gaf_json})
add_library(libgaf::json ALIAS libgaf_json)
target_include_directories(libgaf_json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libgaf_json
    PUBLIC
        libgaf::rapidjson
        libgaf::fmt
        libgaf::gaf
    PRIVATE
        libgaf::project_options
        libgaf::project_warnings
)
