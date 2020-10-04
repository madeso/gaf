SET(GAF_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# based on protobuf cmake buf extended
# https://github.com/Kitware/CMake/blob/master/Modules/FindProtobuf.cmake

function(GAF_GENERATE_CPP)
    set(options IMGUI JSON)
    set(oneValueArgs SOURCES HEADERS)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARG
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    # todo(Gustav): does this work?
    if(NOT ARG_FILES)
        message(SEND_ERROR "Error: GAF_GENERATE_CPP() called without any gaf files")
        return()
    endif()

    find_package(PythonInterp 3 REQUIRED)

    set(SOURCES)
    set(HEADERS)

    foreach(FIL ${ARG_FILES})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        SET(GAF_EXTRA_ARGS "")
        SET(FIL_NAME ${FIL_WE})

        # todo(Gustav): does this work?
        if(${ARG_IMGUI})
            SET(GAF_EXTRA_ARGS "${GAF_EXTRA_ARGS};--include-imgui")
        endif()

        if(${ARG_IMGUI})
            SET(GAF_EXTRA_ARGS "${GAF_EXTRA_ARGS};--include-json")
        endif()

        list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.cc")
        list(APPEND HEADERS "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.h")

        SET(ABSOLUTE_GAF ${GAF_ROOT_DIR}/gaf.py)

        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.cc"
            "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.h"
            COMMAND ${PYTHON_EXECUTABLE}
            ARGS ${ABSOLUTE_GAF} gen ${ABS_FIL} ${CMAKE_CURRENT_BINARY_DIR} ${GAF_EXTRA_ARGS}
            DEPENDS ${ABS_FIL} ${ABSOLUTE_GAF} ${PYTHON_EXECUTABLE} ${GAF_ROOT_DIR}/gaf_cpp.py ${GAF_ROOT_DIR}/gaf_parse.py ${GAF_ROOT_DIR}/gaf_types.py
            COMMENT "Running C++ GAF compiler on ${FIL}"
            VERBATIM
        )
    endforeach()

    set_source_files_properties(${SOURCES} ${HEADERS} PROPERTIES GENERATED TRUE)
    set(${ARG_SOURCES} ${SOURCES} PARENT_SCOPE)
    set(${ARG_HEADERS} ${HEADERS} PARENT_SCOPE)
endfunction()
