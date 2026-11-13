include_guard(GLOBAL)

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

function(workrave_configure_fmt)
  get_target_property(_fmt_target fmt::fmt ALIASED_TARGET)
  if(NOT _fmt_target)
    set(_fmt_target fmt::fmt)
  endif()
  get_target_property(_fmt_includes ${_fmt_target} INTERFACE_INCLUDE_DIRECTORIES)

  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_INCLUDES ${_fmt_includes})
  # Test header parsing without depending on a built fmt library, including
  # when FetchContent has only configured it so far or we are cross-compiling.
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  set(_fmt_probe [=[
    // Ignore a legacy CMAKE_CXX_FLAGS workaround: it does not reach RPC parsing.
    #undef FMT_CONSTEVAL
    #ifdef WORKRAVE_FMT_CONSTEVAL_WORKAROUND
    #define FMT_CONSTEVAL constexpr
    #endif
    #undef FMT_HEADER_ONLY
    #define FMT_HEADER_ONLY 1
    #include <fmt/format.h>
    int main()
    {
      return fmt::format(FMT_STRING("{:08x}"), 42u).empty();
    }
  ]=])

  # Recheck when configuring: headers can change in place after a dependency
  # upgrade, and a previously added target definition must not mask the result.
  unset(WORKRAVE_FMT_CONSTEVAL_COMPILES CACHE)
  check_cxx_source_compiles("${_fmt_probe}" WORKRAVE_FMT_CONSTEVAL_COMPILES)
  if(NOT WORKRAVE_FMT_CONSTEVAL_COMPILES)
    set(CMAKE_REQUIRED_DEFINITIONS -DWORKRAVE_FMT_CONSTEVAL_WORKAROUND)
    unset(WORKRAVE_FMT_CONSTEXPR_COMPILES CACHE)
    check_cxx_source_compiles("${_fmt_probe}" WORKRAVE_FMT_CONSTEXPR_COMPILES)
    if(NOT WORKRAVE_FMT_CONSTEXPR_COMPILES)
      message(FATAL_ERROR "fmt headers do not compile, even with the consteval workaround; see CMakeFiles/CMakeConfigureLog.yaml")
    endif()

    # A usage requirement also reaches clang-rpc-gen's semantic parse context.
    target_compile_definitions(${_fmt_target} INTERFACE FMT_CONSTEVAL=constexpr)
    message(STATUS "fmt: enabling FMT_CONSTEVAL=constexpr for compiler compatibility")
  endif()
  cmake_pop_check_state()
endfunction()
