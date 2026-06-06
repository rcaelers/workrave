# Fix bare dylib cross-references inside a deps directory.
# For each non-symlink dylib, rewrites bare references to other dylibs in the
# same directory to @rpath/ and adds that directory as an LC_RPATH entry.
# Idempotent: already-fixed entries are skipped.
# Usage: cmake -P fixup_deps_dylibs.cmake -- <deps_lib_dir>

set(deps_dir "${CMAKE_ARGV4}")

file(GLOB dylibs "${deps_dir}/*.dylib")
foreach(dylib ${dylibs})
  if(IS_SYMLINK "${dylib}")
    continue()
  endif()

  execute_process(COMMAND otool -L "${dylib}" OUTPUT_VARIABLE lc_load)
  set(fixed FALSE)
  string(REPLACE "\n" ";" lines "${lc_load}")
  foreach(line ${lines})
    if(line MATCHES "^\t([^/@\t][^ \t]+\\.dylib)")
      set(libname "${CMAKE_MATCH_1}")
      if(EXISTS "${deps_dir}/${libname}")
        execute_process(COMMAND install_name_tool -change "${libname}" "@rpath/${libname}" "${dylib}")
        set(fixed TRUE)
      endif()
    endif()
  endforeach()

  if(fixed)
    execute_process(COMMAND otool -l "${dylib}" OUTPUT_VARIABLE lc_all)
    if(NOT lc_all MATCHES "path ${deps_dir} ")
      execute_process(COMMAND install_name_tool -add_rpath "${deps_dir}" "${dylib}")
    endif()
  endif()
endforeach()
