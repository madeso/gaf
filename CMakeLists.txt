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
    set(options)
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
    # message(STATUS "Base: ${ARG_BASE_NAME} for ${ARG_CC_NAME}")
    # {custom_underscore}
    GAF_GENERATE_CPP(GAF_SOURCES GAF_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/examples/${ARG_BASE_NAME}.c)
    add_executable(${ARG_BASE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/cpp_samples/${ARG_CC_NAME} ${GAF_SOURCES} ${GAF_HEADERS})
    target_include_directories(${ARG_BASE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${ARG_BASE_NAME} PUBLIC catch)
endfunction()

add_sample(BASE_NAME defaultvalues)
add_sample(BASE_NAME constants)
add_sample(BASE_NAME comments1)
add_sample(BASE_NAME comments2)
add_sample(BASE_NAME comments3)
add_sample(BASE_NAME underscore)
add_sample(BASE_NAME package)
# add_sample(twostructs, json_test=True),
# add_sample(master, json_test=True),
# add_sample(enum, json_test=True)
