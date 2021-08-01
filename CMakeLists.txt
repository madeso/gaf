# this is only for examples and testing
cmake_minimum_required(VERSION 3.1)
project(gaf CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/modules")

include(tools/cmake/standard_project_settings.cmake)
include(tools/cmake/compiler_warnings.cmake)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

include(gaf.cmake)
add_subdirectory(external)
add_subdirectory(src)

include(CTest)
include(external/catch/Catch.cmake)


function(add_sample)
    set(options RAPIDJSON PUGIXML)
    set(oneValueArgs BASE_NAME CC_NAME)
    set(multiValueArgs)
    cmake_parse_arguments(ARG
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )
    if("${ARG_CC_NAME}" STREQUAL "")
        set(ARG_CC_NAME ${ARG_BASE_NAME}.cc)
    endif()

    set(APP_NAME ${ARG_BASE_NAME})
    if(${ARG_RAPIDJSON})
        set(APP_NAME ${APP_NAME}_json)
    endif()
    if(${ARG_PUGIXML})
        set(APP_NAME ${APP_NAME}_xml)
    endif()

    GAF_GENERATE_CPP(SOURCES GAF_SOURCES HEADERS GAF_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/gaf/${ARG_BASE_NAME}.c)
    set(src ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/${ARG_CC_NAME} ${GAF_SOURCES} ${GAF_HEADERS})

    if(${ARG_RAPIDJSON})
        GAF_GENERATE_CPP(RAPIDJSON SOURCES GAF_JSON_SOURCES HEADERS GAF_JSON_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/gaf/${ARG_BASE_NAME}.c)
        set(src ${src} ${GAF_JSON_SOURCES} ${GAF_JSON_HEADERS})
    endif()
    if(${ARG_PUGIXML})
        GAF_GENERATE_CPP(PUGIXML SOURCES GAF_JSON_SOURCES HEADERS GAF_JSON_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/gaf/${ARG_BASE_NAME}.c)
        set(src ${src} ${GAF_JSON_SOURCES} ${GAF_JSON_HEADERS})
    endif()

    add_executable(${APP_NAME} ${src})
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${APP_NAME} PUBLIC catch PRIVATE project_options project_warnings)

    if(${ARG_RAPIDJSON})
        target_link_libraries(${APP_NAME} PUBLIC rapidjson)
        target_compile_definitions(${APP_NAME} PUBLIC -DGAF_TEST_JSON)
    endif()
    if(${ARG_PUGIXML})
        target_link_libraries(${APP_NAME} PUBLIC pugixml fmt)
        target_compile_definitions(${APP_NAME} PUBLIC -DGAF_TEST_XML)
    endif()

    catch_discover_tests(${APP_NAME})
endfunction()

add_sample(BASE_NAME defaultvalues)
add_sample(BASE_NAME constants)
add_sample(BASE_NAME comments1)
add_sample(BASE_NAME comments2)
add_sample(BASE_NAME comments3)
add_sample(BASE_NAME underscore)
add_sample(BASE_NAME package)

add_sample(BASE_NAME twostructs)
add_sample(BASE_NAME twostructs RAPIDJSON)
add_sample(BASE_NAME twostructs PUGIXML)

add_sample(BASE_NAME master)
add_sample(BASE_NAME master RAPIDJSON)
add_sample(BASE_NAME master PUGIXML)

add_sample(BASE_NAME enum)
add_sample(BASE_NAME enum RAPIDJSON)
add_sample(BASE_NAME enum PUGIXML)
