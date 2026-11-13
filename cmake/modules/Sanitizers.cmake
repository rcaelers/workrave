# Named sanitizer configurations use Debug flags and existing dependency builds.
if(DEFINED CACHE{CMAKE_BUILD_TYPE})
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel ASan UBSan ASanUBSan)
endif()

set(_workrave_sanitizer_compiler OFF)
if(NOT MSVC AND CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$"
            AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  set(_workrave_sanitizer_compiler ON)
endif()

get_property(_workrave_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_workrave_multi_config AND _workrave_sanitizer_compiler)
  list(APPEND CMAKE_CONFIGURATION_TYPES ASan UBSan ASanUBSan)
  list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Available build configurations" FORCE)
endif()

foreach(_config ASan UBSan ASanUBSan)
  string(TOUPPER "${_config}" _upper_config)
  foreach(_flags CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS)
    if(NOT DEFINED ${_flags}_${_upper_config} OR "${${_flags}_${_upper_config}}" STREQUAL "")
      set(${_flags}_${_upper_config} "${${_flags}_DEBUG}" CACHE STRING "${_config} flags" FORCE)
    endif()
    mark_as_advanced(${_flags}_${_upper_config})
  endforeach()
  # Imported dependencies need not provide separately instrumented binaries.
  # Debian/Ubuntu exports use NONE; other packages use NOCONFIG or a
  # configuration-independent IMPORTED_LOCATION (the empty entry).
  if(NOT DEFINED CMAKE_MAP_IMPORTED_CONFIG_${_upper_config})
    set(CMAKE_MAP_IMPORTED_CONFIG_${_upper_config} Debug RelWithDebInfo Release MinSizeRel NONE NOCONFIG "")
  endif()
endforeach()

set(_workrave_sanitizer_description "none")
set(_workrave_required_sanitizers "")
if(_workrave_multi_config)
  foreach(_config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${_config}" _upper_config)
    if(_upper_config STREQUAL "ASAN")
      list(APPEND _workrave_required_sanitizers address)
    elseif(_upper_config STREQUAL "UBSAN")
      list(APPEND _workrave_required_sanitizers undefined)
    elseif(_upper_config STREQUAL "ASANUBSAN")
      list(APPEND _workrave_required_sanitizers "address,undefined")
    endif()
  endforeach()
  if(_workrave_required_sanitizers)
    set(_workrave_sanitizer_description "per configuration (ASan/UBSan/ASanUBSan)")
  endif()
else()
  string(TOUPPER "${CMAKE_BUILD_TYPE}" _upper_build_type)
  if(_upper_build_type STREQUAL "ASAN")
    list(APPEND _workrave_required_sanitizers address)
    set(_workrave_sanitizer_description address)
  elseif(_upper_build_type STREQUAL "UBSAN")
    list(APPEND _workrave_required_sanitizers undefined)
    set(_workrave_sanitizer_description undefined)
  elseif(_upper_build_type STREQUAL "ASANUBSAN")
    list(APPEND _workrave_required_sanitizers "address,undefined")
    set(_workrave_sanitizer_description "address,undefined")
  endif()
endif()
list(REMOVE_DUPLICATES _workrave_required_sanitizers)
if(NOT _workrave_required_sanitizers)
  return()
endif()
if(NOT _workrave_sanitizer_compiler)
  message(FATAL_ERROR "Sanitizer builds require GCC or Clang with the GNU-style command line")
endif()

include(CMakePushCheckState)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
foreach(_sanitizer IN LISTS _workrave_required_sanitizers)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "-fsanitize=${_sanitizer}")
  string(TOUPPER "${_sanitizer}" _upper_sanitizer)
  string(REPLACE "," "_" _upper_sanitizer "${_upper_sanitizer}")
  check_c_source_compiles("int main(void) { return 0; }" WORKRAVE_C_SANITIZER_${_upper_sanitizer})
  check_cxx_source_compiles("int main() { return 0; }" WORKRAVE_CXX_SANITIZER_${_upper_sanitizer})
  cmake_pop_check_state()
  if(NOT WORKRAVE_C_SANITIZER_${_upper_sanitizer} OR NOT WORKRAVE_CXX_SANITIZER_${_upper_sanitizer})
    message(FATAL_ERROR "The C/C++ toolchain cannot compile and link with -fsanitize=${_sanitizer}; check its sanitizer runtime installation")
  endif()
endforeach()

# Also used by tools that compile/link helper executables outside CMake.
set(_workrave_sanitizer_link_flags
  "$<$<CONFIG:ASan>:-fsanitize=address>$<$<CONFIG:UBSan>:-fsanitize=undefined>$<$<CONFIG:ASanUBSan>:-fsanitize=address,undefined>")
add_link_options("${_workrave_sanitizer_link_flags}")

add_compile_options(
  "${_workrave_sanitizer_link_flags}"
  "$<$<CONFIG:ASan,UBSan,ASanUBSan>:-fno-omit-frame-pointer>"
  "$<$<CONFIG:ASan,UBSan,ASanUBSan>:-fno-optimize-sibling-calls>"
  # Findings fail tests without requiring UBSAN_OPTIONS in the environment.
  "$<$<CONFIG:UBSan,ASanUBSan>:-fno-sanitize-recover=undefined>")
