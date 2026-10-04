function(resolve_dependencies target dependencies_out resolved_dependencies_out dependencies_search_dirs)

  set(new_dependencies)

  execute_process(COMMAND ${CMAKE_OBJDUMP} "-p" ${target} OUTPUT_VARIABLE objdump_output
                  COMMAND_ERROR_IS_FATAL ANY)

  string(REGEX REPLACE ";" "\\\\;" lines "${objdump_output}")
  string(REGEX REPLACE "\n" ";" lines "${lines}")

  foreach(line ${lines})
    set(regex "^[ \t]*DLL Name: (.*\\.[Dd][Ll][Ll])$")

    if("${line}" MATCHES "${regex}")
      string(REGEX REPLACE ${regex} "\\1" item "${line}")

      list(FIND ${dependencies_out} ${item} exists)
      if(exists EQUAL -1)
        list(APPEND ${dependencies_out} ${item})
        set(path "path-NOTFOUND")
        find_file(path "${item}" PATHS ${dependencies_search_dirs} NO_DEFAULT_PATH NO_CACHE)

        if(NOT path STREQUAL "path-NOTFOUND")
          list(APPEND new_dependencies "${path}")
          list(APPEND ${resolved_dependencies_out} ${path})
        else()
          if(STRICT_WINDOWS_DEPENDENCIES)
            string(TOLOWER "${item}" lower_item)
            string(REGEX REPLACE "\\.dll$" "" system_library "${lower_item}")
            # Windows SDK import libraries identify OS-provided DLLs. The
            # MinGW C++/thread runtimes must always be bundled.
            if(NOT lower_item MATCHES "^(api-ms-|ext-ms-)" AND
               (NOT EXISTS "${SYS_ROOT}/lib/lib${system_library}.a" OR
                lower_item MATCHES "^(libc\\+\\+|libunwind|libwinpthread).*"))
              message(FATAL_ERROR "Missing runtime dependency ${item} imported by ${target}")
            endif()
          endif()
          message(STATUS "System library: ${item}")
        endif()
      endif()
    endif()
  endforeach()

  set(tmp ${new_dependencies})
  foreach(dependency ${tmp})
    resolve_dependencies("${dependency}" ${dependencies_out} ${resolved_dependencies_out} "${dependencies_search_dirs}")
  endforeach()
  
  set(${dependencies_out} ${${dependencies_out}} PARENT_SCOPE)
  set(${resolved_dependencies_out} ${${resolved_dependencies_out}} PARENT_SCOPE)
endfunction()
