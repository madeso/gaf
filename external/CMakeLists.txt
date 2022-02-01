if(GAF_MASTER_PROJECT)
    message(STATUS "Adding gaf master dependencies")

    add_library(catch INTERFACE)
    target_include_directories(catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/catch)
    add_library(libgaf::catch ALIAS catch)

    add_library(rapidjson INTERFACE)
    target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson-master/include)
    add_library(libgaf::rapidjson ALIAS rapidjson)
endif()

add_library(gaf_fmt STATIC
    # fmt-8.0.1/src/fmt.cc
    fmt-8.0.1/src/format.cc
    fmt-8.0.1/src/os.cc
)
add_library(libgaf::fmt ALIAS gaf_fmt)
target_include_directories(gaf_fmt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/fmt-8.0.1/include)
target_compile_features(gaf_fmt PRIVATE cxx_std_17)

set(pugi_src ${CMAKE_CURRENT_SOURCE_DIR}/pugixml-1.11.4/src)
add_library(gaf_pugixml STATIC
    ${pugi_src}/pugiconfig.hpp
    ${pugi_src}/pugixml.hpp
    ${pugi_src}/pugixml.cpp
)
target_include_directories(gaf_pugixml PUBLIC ${pugi_src})
target_compile_features(gaf_pugixml PRIVATE cxx_std_11)
add_library(libgaf::pugixml ALIAS gaf_pugixml)
