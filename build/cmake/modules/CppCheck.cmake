if (NOT CPPCHECK_FOUND)
  find_package(cppcheck QUIET)
endif()

add_custom_target(all_cppcheck)
set_target_properties(all_cppcheck PROPERTIES EXCLUDE_FROM_ALL TRUE)

function(add_cppcheck target)
  if(CPPCHECK_FOUND)
    set(cppcheck_args "--enable=warning,style,portability,information")

    get_target_property(_sources "${target}" SOURCES)
    set(files)
    foreach(_source ${_sources})
      get_source_file_property(_language "${_source}" LANGUAGE)
      get_source_file_property(_location "${_source}" LOCATION)
      get_source_file_property(_generated "${_source}" GENERATED)
      if (("${_language}" MATCHES "CXX") AND (NOT _generated))
	list(APPEND files "${_location}")
      endif()
    endforeach()

    get_target_property(_includes "${target}" INCLUDE_DIRECTORIES)
    set(includes)
    foreach(_include ${_includes})
      if ("${_include}" MATCHES "^${CMAKE_SOURCE_DIR}/")
        list(APPEND includes "-I${_include}")
      endif()
    endforeach()

    set(flags)
    get_target_property(value ${target} COMPILE_DEFINITIONS)
    if (value)
      foreach(item ${value})
        list(APPEND flags "-D${item}")
      endforeach()
    endif()
    STRING(TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" name)
    get_target_property(value ${target} ${name})
    if (value)
      foreach(item ${value})
        list(APPEND flags "-D${item}")
      endforeach()
    endif()
    get_directory_property(value DEFINITIONS)
    if (value)
      list(APPEND flags ${value})
    endif()
    get_target_property(__definitions "${target}" COMPILE_DEFINITIONS)

    add_custom_target(all_cppcheck_${target})
    set_target_properties(all_cppcheck_${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    add_custom_command(TARGET
      all_cppcheck_${target}
      PRE_BUILD
      COMMAND
      ${CPPCHECK_EXECUTABLE}
      ${CPPCHECK_QUIET_ARG}
      ${CPPCHECK_TEMPLATE_ARG}
      ${cppcheck_args}
      ${includes}
      ${flags}
      ${files}
      WORKING_DIRECTORY
      "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMENT
      "all_cppcheck_${target}: Running cppcheck on target ${target}."
      VERBATIM)

    add_dependencies(all_cppcheck all_cppcheck_${target})
  endif()
endfunction()
