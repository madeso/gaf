if(GAF_MASTER_PROJECT)
    message(STATUS "Adding gaf master dependencies")

    # =========================================================================
    # catch
    add_library(catch INTERFACE)
    target_include_directories(catch SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/catch)
    add_library(libgaf::catch ALIAS catch)    
endif()



# =============================================================================
# rapid json
add_library(gaf_rapidjson INTERFACE)
target_include_directories(gaf_rapidjson SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson-master/include)
add_library(libgaf::rapidjson ALIAS gaf_rapidjson)




# =============================================================================
# fmt
add_library(gaf_fmt STATIC
    # fmt-9.1.0/src/fmt.cc
    fmt-9.1.0/src/format.cc
    fmt-9.1.0/src/os.cc
)
target_include_directories(gaf_fmt SYSTEM
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/fmt-9.1.0/include
)
target_compile_features(gaf_fmt PRIVATE cxx_std_20)
add_library(libgaf::fmt ALIAS gaf_fmt)



# =============================================================================
# pugi xml
set(pugi_src ${CMAKE_CURRENT_SOURCE_DIR}/pugixml-1.11.4/src)
add_library(gaf_pugixml STATIC
    ${pugi_src}/pugiconfig.hpp
    ${pugi_src}/pugixml.hpp
    ${pugi_src}/pugixml.cpp
)
target_include_directories(gaf_pugixml SYSTEM PUBLIC ${pugi_src})
target_compile_features(gaf_pugixml PRIVATE cxx_std_11)
add_library(libgaf::pugixml ALIAS gaf_pugixml)
