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
    set(options RAPIDJSON)
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

    GAF_GENERATE_CPP(SOURCES GAF_SOURCES HEADERS GAF_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/${ARG_BASE_NAME}.c)
    set(src ${CMAKE_CURRENT_SOURCE_DIR}/cpp_samples/${ARG_CC_NAME} ${GAF_SOURCES} ${GAF_HEADERS})

    if(${ARG_RAPIDJSON})
        GAF_GENERATE_CPP(RAPIDJSON SOURCES GAF_JSON_SOURCES HEADERS GAF_JSON_HEADERS FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/${ARG_BASE_NAME}.c)
        set(src ${src} ${GAF_JSON_SOURCES} ${GAF_JSON_HEADERS})
    endif()

    add_executable(${APP_NAME} ${src})
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${APP_NAME} PUBLIC catch)

    if(${ARG_RAPIDJSON})
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
add_sample(RAPIDJSON BASE_NAME twostructs)

add_sample(BASE_NAME master)
add_sample(RAPIDJSON BASE_NAME master)

add_sample(BASE_NAME enum)
add_sample(RAPIDJSON BASE_NAME enum)
