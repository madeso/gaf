# this is only for examples and testing
cmake_minimum_required(VERSION 3.1)
project(gaf)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# SET(GAF_TEST_JSON {json})

include(gaf.cmake)
add_subdirectory(cpp_externals)

function(add_sample)
    set(options JSON)
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
    if(${ARG_JSON})
        set(APP_NAME ${APP_NAME}_json)
    endif()

    # todo(Gustav): split imgui and json to seperate commands

    if(${ARG_JSON})
        GAF_GENERATE_CPP(JSON SOURCES GAF_SOURCES HEADERS GAF_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/${ARG_BASE_NAME}.c)
    else()
        GAF_GENERATE_CPP(SOURCES GAF_SOURCES HEADERS GAF_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/${ARG_BASE_NAME}.c)
    endif()

    add_executable(${APP_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/cpp_samples/${ARG_CC_NAME} ${GAF_SOURCES} ${GAF_HEADERS})
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${APP_NAME} PUBLIC catch)

    if(${ARG_JSON})
        target_link_libraries(${APP_NAME} PUBLIC rapidjson)
        target_compile_definitions(${APP_NAME} PUBLIC -DGAF_TEST_JSON)
    endif()
endfunction()

add_sample(BASE_NAME defaultvalues)
add_sample(BASE_NAME constants)
add_sample(BASE_NAME comments1)
add_sample(BASE_NAME comments2)
add_sample(BASE_NAME comments3)
add_sample(BASE_NAME underscore)
add_sample(BASE_NAME package)

add_sample(BASE_NAME twostructs)
add_sample(JSON BASE_NAME twostructs)

add_sample(BASE_NAME master)
add_sample(JSON BASE_NAME master)

add_sample(BASE_NAME enum)
add_sample(JSON BASE_NAME enum)
