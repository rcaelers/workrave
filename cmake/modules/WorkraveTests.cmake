if(APPLE AND TARGET Boost::unit_test_framework)
  get_target_property(_wrt_boost_loc Boost::unit_test_framework IMPORTED_LOCATION_RELEASE)
  if(NOT _wrt_boost_loc)
    get_target_property(_wrt_boost_loc Boost::unit_test_framework IMPORTED_LOCATION_DEBUG)
  endif()
  if(_wrt_boost_loc)
    cmake_path(GET _wrt_boost_loc PARENT_PATH _wrt_deps_lib_dir)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/fixup_deps_dylibs.cmake"
              -- "${_wrt_deps_lib_dir}"
    )
  endif()
endif()

function(workrave_add_test target)
  add_test(NAME ${target} COMMAND ${target})

  # Each test binary gets its own WORKRAVE_HOME so parallel ctest runs don't
  # share the stats/config directory (~/.workrave-qt/).
  set(_env "WORKRAVE_HOME=$<TARGET_FILE_DIR:${target}>")

  if(APPLE)
    # Rewrite bare dylib references (e.g. Boost) to @rpath/ post-build so the
    # binary runs without DYLD_LIBRARY_PATH when invoked directly or via ctest.
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/fixup_bare_dylib_refs.cmake"
              -- "$<TARGET_FILE:${target}>"
      COMMENT "Fixing bare dylib references in ${target}"
      VERBATIM
    )
  else()
    list(APPEND _env "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_BINARY_DIR}/Frameworks:$ENV{LD_LIBRARY_PATH}")
  endif()

  set_tests_properties(${target} PROPERTIES ENVIRONMENT "${_env}")
endfunction()
