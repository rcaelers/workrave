# IWYU.cmake — include-what-you-use integration
#
# Provides target_enable_iwyu(<target>) to attach IWYU analysis to a target.
# Enable with: cmake -DWITH_IWYU=ON
#
# Bundled mapping files from the IWYU installation are auto-detected.
# Project-local mappings under cmake/iwyu/ (qt.imp, boost.imp, spdlog.imp) are
# also loaded when present.

option(WITH_IWYU "Enable include-what-you-use static analysis" OFF)

if(NOT WITH_IWYU)
  macro(target_enable_iwyu)
  endmacro()
  return()
endif()

find_program(IWYU_EXECUTABLE
  NAMES include-what-you-use iwyu
  DOC "Path to the include-what-you-use executable"
)

if(NOT IWYU_EXECUTABLE)
  message(WARNING "WITH_IWYU=ON but include-what-you-use not found — disabling IWYU")
  macro(target_enable_iwyu)
  endmacro()
  return()
endif()

# Locate the bundled mapping files shipped alongside the binary
get_filename_component(_iwyu_bindir "${IWYU_EXECUTABLE}" DIRECTORY)
find_path(IWYU_SHARE_DIR
  NAMES qt5_11.imp
  HINTS
    "${_iwyu_bindir}/../share/include-what-you-use"
    "/usr/share/include-what-you-use"
    "/usr/local/share/include-what-you-use"
  DOC "Directory containing IWYU bundled mapping files"
)

# Generate a Python wrapper that:
#   - silently skips Objective-C++ files (IWYU aborts on ObjC)
#   - strips the verbose "full include-list" summary blocks from output
set(_iwyu_wrapper "${CMAKE_BINARY_DIR}/cmake/iwyu/iwyu_wrapper.py")
configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/iwyu/iwyu_wrapper.py.in"
  "${_iwyu_wrapper}"
  @ONLY
)
file(CHMOD "${_iwyu_wrapper}" PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE
  WORLD_READ WORLD_EXECUTE)

# Base command — each -Xiwyu flag and its argument must be separate list elements
set(_iwyu_cmd
  "${_iwyu_wrapper}"
  -Xiwyu --cxx17ns
  -Xiwyu --no_fwd_decls
  -Xiwyu --quoted_includes_first
  -Xiwyu --keep=*/config.h
)

if(IWYU_SHARE_DIR)
  # qt5_11.imp covers all Qt6 public header names (they didn't change between Qt5/Qt6)
  foreach(_imp qt5_11.imp boost-all.imp boost-all-private.imp)
    if(EXISTS "${IWYU_SHARE_DIR}/${_imp}")
      list(APPEND _iwyu_cmd -Xiwyu "--mapping_file=${IWYU_SHARE_DIR}/${_imp}")
    endif()
  endforeach()
  message(STATUS "IWYU: using bundled mappings from ${IWYU_SHARE_DIR}")
else()
  message(WARNING "IWYU: bundled mapping files not found; Qt/Boost mappings unavailable")
endif()

foreach(_imp qt.imp boost.imp spdlog.imp workrave.imp)
  set(_imp_path "${CMAKE_SOURCE_DIR}/cmake/iwyu/${_imp}")
  if(EXISTS "${_imp_path}")
    list(APPEND _iwyu_cmd -Xiwyu "--mapping_file=${_imp_path}")
    message(STATUS "IWYU: loading project mapping ${_imp_path}")
  endif()
endforeach()

# On macOS, IWYU uses its own Homebrew clang which doesn't automatically pick up
# the Xcode SDK headers (type_traits, etc.). Pass the SDK path explicitly.
if(APPLE)
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE _macos_sdk_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _xcrun_result
  )
  if(_xcrun_result EQUAL 0 AND _macos_sdk_path)
    list(APPEND _iwyu_cmd -isysroot "${_macos_sdk_path}")
    message(STATUS "IWYU: macOS sysroot ${_macos_sdk_path}")
  else()
    message(WARNING "IWYU: xcrun failed to find SDK path; standard headers may be missing")
  endif()
endif()

set(IWYU_COMMAND "${_iwyu_cmd}" CACHE INTERNAL "IWYU invocation command" FORCE)

# Apply globally — every C++ target defined after this point inherits IWYU.
# Objective-C++ sources that IWYU cannot handle must opt out with SKIP_LINTING ON.
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_COMMAND}")

message(STATUS "IWYU: enabled globally (${IWYU_EXECUTABLE})")

# Kept for explicit per-target use (e.g. re-enabling after a set_target_properties override).
macro(target_enable_iwyu _target)
  set_target_properties(${_target} PROPERTIES
    CXX_INCLUDE_WHAT_YOU_USE "${IWYU_COMMAND}"
  )
endmacro()
