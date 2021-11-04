if (HAVE_TESTS)
  add_executable(workrave-config-test
    SimulatedTime.cc
    ConfigTests.cc)
  target_code_coverage(workrave-config-test AUTO)

  target_link_libraries(workrave-config-test PRIVATE workrave-libs-config)
  target_link_libraries(workrave-config-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-config-test PRIVATE ${Boost_LIBRARIES})
  target_include_directories(workrave-config-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/config/src)
  target_compile_definitions(workrave-config-test PRIVATE -DBUILDDIR="${CMAKE_CURRENT_BINARY_DIR}")

  if (HAVE_GSETTINGS)
    target_link_libraries(workrave-config-test PRIVATE ${GLIB_LIBRARIES})
    target_include_directories(workrave-config-test PRIVATE ${GLIB_INCLUDE_DIRS})
    add_custom_target(
        config_test_schema ALL
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.test.gschema.xml
                ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.test.gschema.xml
        COMMAND ${glib_schema_compiler} ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if (PLATFORM_OS_MACOS)
    target_include_directories(workrave-config-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/config/src/macos)
  endif()

  add_test(NAME workrave-config-test COMMAND workrave-config-test)
endif()
