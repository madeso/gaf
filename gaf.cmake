SET(GAF_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# stolen from protobuf https://github.com/Kitware/CMake/blob/master/Modules/FindProtobuf.cmake
function(GAF_GENERATE_CPP SRCS HDRS)
    if(NOT ARGN)
        message(SEND_ERROR "Error: GAF_GENERATE_CPP() called without any gaf files")
        return()
    endif()

    find_package(PythonInterp 3 REQUIRED)

    set(${SRCS})
    set(${HDRS})
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        SET(GAF_EXTRA_ARGS "")
        SET(FIL_NAME ${FIL_WE})

        SET(GAF_EXTRA_DEP)

        if(DEFINED Gaf_CUSTOM_ARGUMENTS_FROM_FILE)
            SET(GAF_EXTRA_ARGS "${GAF_EXTRA_ARGS};@${Gaf_CUSTOM_ARGUMENTS_FROM_FILE}")
            SET(GAF_EXTRA_DEP "${Gaf_CUSTOM_ARGUMENTS_FROM_FILE}")
        endif()
        if(DEFINED Gaf_INCLUDE_IMGUI)
            SET(GAF_EXTRA_ARGS "${GAF_EXTRA_ARGS};--include-imgui")
        endif()

        list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.cc")
        list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.h")
        # message(STATUS "Appending ${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.h")

        SET(ABSOLUTE_GAF ${GAF_ROOT_DIR}/gaf.py)

        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.cc"
                    "${CMAKE_CURRENT_BINARY_DIR}/gaf_${FIL_NAME}.h"
            COMMAND  ${PYTHON_EXECUTABLE}
            ARGS ${ABSOLUTE_GAF} gen ${ABS_FIL} ${CMAKE_CURRENT_BINARY_DIR} ${GAF_EXTRA_ARGS}
            DEPENDS ${ABS_FIL} ${ABSOLUTE_GAF} ${PYTHON_EXECUTABLE} ${GAF_EXTRA_DEP} ${GAF_ROOT_DIR}/gaf_cpp.py ${GAF_ROOT_DIR}/gaf_parse.py ${GAF_ROOT_DIR}/gaf_types.py
            COMMENT "Running C++ GAF compiler on ${FIL}"
            VERBATIM
        )
    endforeach()

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)

    # message(STATUS "headers ${HDRS}")
    # message(STATUS "hh ${${HDRS}}")
endfunction()
