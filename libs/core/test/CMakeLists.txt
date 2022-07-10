if (HAVE_TESTS)
  add_executable(workrave-core-timer-test
    SimulatedTime.cc
    TimerTests.cc)
  target_code_coverage(workrave-core-timer-test AUTO)

  target_link_libraries(workrave-core-timer-test PRIVATE workrave-libs-core)
  target_link_libraries(workrave-core-timer-test PRIVATE Boost::test_exec_monitor)
  target_link_libraries(workrave-core-timer-test PRIVATE ${EXTRA_LIBRARIES})
  set_target_properties(workrave-core-timer-test PROPERTIES USE_STUBS ON)

  target_include_directories(workrave-core-timer-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/core/src)

  add_executable(workrave-core-integration-test
    ActivityMonitorStub.cc
    IntegrationTests.cc
    SimulatedTime.cc
    )
  target_code_coverage(workrave-core-integration-test AUTO)

  set_target_properties(workrave-core-integration-test PROPERTIES USE_STUBS ON)

  if (HAVE_APP_GTK)
    target_link_directories(workrave-core-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  target_link_libraries(workrave-core-integration-test PRIVATE workrave-libs-core)
  target_link_libraries(workrave-core-integration-test PRIVATE workrave-libs-config)
  target_link_libraries(workrave-core-integration-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-core-integration-test PRIVATE workrave-libs-dbus-stub)
  target_link_libraries(workrave-core-integration-test PRIVATE Boost::test_exec_monitor)
  target_link_libraries(workrave-core-integration-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-integration-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/core/src)

  if (HAVE_APP_QT)
    target_link_libraries(workrave-core-integration-test PRIVATE ${Qt6DBus_LIBRARIES})
    target_link_libraries(workrave-core-integration-test PRIVATE ${Qt6Widgets_LIBRARIES})
  endif()
  if (HAVE_APP_GTK OR HAVE_GLIB)
    target_link_libraries(workrave-core-integration-test PRIVATE ${GLIB_LIBRARIES})
    target_link_directories(workrave-core-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  if (PLATFORM_OS_UNIX)
    target_link_libraries(workrave-core-integration-test PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
  endif()

  if (SSP_LIBRARY)
    target_link_libraries(workrave-core-integration-test PRIVATE ${SSP_LIBRARY})
    target_link_libraries(workrave-core-timer-test PRIVATE ${SSP_LIBRARY})
  endif()

  add_test(NAME workrave-core-integration-test COMMAND workrave-core-integration-test)
  add_test(NAME workrave-core-timer-test COMMAND workrave-core-timer-test)

  set_tests_properties(workrave-core-integration-test PROPERTIES
    ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_BINARY_DIR}/Frameworks:$ENV{LD_LIBRARY_PATH})
  set_tests_properties(workrave-core-timer-test PROPERTIES
    ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_BINARY_DIR}/Frameworks:$ENV{LD_LIBRARY_PATH})

endif()
