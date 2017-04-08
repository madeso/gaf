SET(GAF_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

# stolen from protobuf https://github.com/Kitware/CMake/blob/master/Modules/FindProtobuf.cmake
function(GAF_GENERATE_CPP SRCS HDRS)
  if(NOT ARGN)
    message(SEND_ERROR "Error: GAF_GENERATE_CPP() called without any gaf files")
    return()
  endif()

  find_package( PythonInterp 3 REQUIRED )

  if(GAF_GENERATE_CPP_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FIL ${ARGN})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(FIND _gaf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _gaf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  else()
    set(_gaf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if(DEFINED GAF_IMPORT_DIRS AND NOT DEFINED Gaf_IMPORT_DIRS)
    set(Gaf_IMPORT_DIRS "${GAF_IMPORT_DIRS}")
  endif()

  if(DEFINED Gaf_IMPORT_DIRS)
    foreach(DIR ${Gaf_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(FIND _gaf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _gaf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  endif()

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    if(NOT GAF_GENERATE_CPP_APPEND_PATH)
      get_filename_component(FIL_DIR ${FIL} DIRECTORY)
      if(FIL_DIR)
        set(FIL_WE "${FIL_DIR}/${FIL_WE}")
      endif()
    endif()

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.cc")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.h")

    SET(ABSOLUTE_GAF ${GAF_ROOT_DIR}/gaf.py)

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.cc"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.h"
      COMMAND  ${PYTHON_EXECUTABLE}
      ARGS ${ABSOLUTE_GAF} gen cpp ${ABS_FIL} ${CMAKE_CURRENT_BINARY_DIR} --name ${FIL_WE}
      DEPENDS ${ABS_FIL} ${ABSOLUTE_GAF} ${PYTHON_EXECUTABLE}
      COMMENT "Running C++ GAF compiler on ${FIL}"
      VERBATIM)
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()
