SET(GAF_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# based on protobuf cmake buf extended
# https://github.com/Kitware/CMake/blob/master/Modules/FindProtobuf.cmake

function(GAF_GENERATE_CPP)
    set(options IMGUI RAPIDJSON PUGIXML)
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

    set(SOURCES)
    set(HEADERS)

    foreach(FIL ${ARG_FILES})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        SET(GAF_EXTENSION_ARG "cpp")
        SET(FIL_NAME ${FIL_WE})

        SET(GAF_PREFIX "gaf_")

        # todo(Gustav): does this work?
        if(${ARG_IMGUI})
            SET(GAF_EXTENSION_ARG "imgui")
            SET(GAF_PREFIX "gaf_imgui_")
        endif()

        if(${ARG_RAPIDJSON})
            SET(GAF_EXTENSION_ARG "rapidjson")
            SET(GAF_PREFIX "gaf_rapidjson_")
        endif()
        if(${ARG_PUGIXML})
            SET(GAF_EXTENSION_ARG "pugixml")
            SET(GAF_PREFIX "gaf_pugixml_")
        endif()

        list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${GAF_PREFIX}${FIL_NAME}.cc")
        list(APPEND HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${GAF_PREFIX}${FIL_NAME}.h")

        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${GAF_PREFIX}${FIL_NAME}.cc"
            "${CMAKE_CURRENT_BINARY_DIR}/${GAF_PREFIX}${FIL_NAME}.h"
            COMMAND gaf
            ARGS generate ${ABS_FIL} ${CMAKE_CURRENT_BINARY_DIR} ${GAF_EXTENSION_ARG}
            DEPENDS ${ABS_FIL} gaf
            COMMENT "Running C++ GAF compiler on ${FIL}"
            VERBATIM
        )
    endforeach()

    set_source_files_properties(${SOURCES} ${HEADERS} PROPERTIES GENERATED TRUE)
    set(${ARG_SOURCES} ${SOURCES} PARENT_SCOPE)
    set(${ARG_HEADERS} ${HEADERS} PARENT_SCOPE)
endfunction()
