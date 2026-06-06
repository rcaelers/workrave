# Rewrite bare dylib references (no leading / or @) to @rpath/ so the binary
# can be run directly without setting DYLD_LIBRARY_PATH.
# Usage: cmake -P fixup_bare_dylib_refs.cmake -- <binary>
set(binary "${CMAKE_ARGV4}")
execute_process(COMMAND otool -L "${binary}" OUTPUT_VARIABLE output OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "\n" ";" lines "${output}")
foreach(line ${lines})
  if(line MATCHES "^\t([^/@\t][^ \t]+\\.dylib)")
    set(libname "${CMAKE_MATCH_1}")
    execute_process(
      COMMAND install_name_tool -change "${libname}" "@rpath/${libname}" "${binary}"
      RESULT_VARIABLE result
    )
    if(result EQUAL 0)
      message(STATUS "Fixed rpath: ${libname} -> @rpath/${libname}")
    endif()
  endif()
endforeach()
