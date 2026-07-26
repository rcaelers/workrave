if (HAVE_TESTS)
  add_executable(workrave-core-next-timer-test
    SimulatedTime.cc
    TimerTests.cc)
  target_code_coverage(workrave-core-next-timer-test AUTO)

  set_target_properties(workrave-core-next-timer-test PROPERTIES USE_STUBS ON)

  target_link_libraries(workrave-core-next-timer-test PRIVATE workrave-libs-core-next)
  target_link_libraries(workrave-core-next-timer-test PRIVATE Boost::unit_test_framework)
  target_link_libraries(workrave-core-next-timer-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-next-timer-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/corenext/src)

  add_executable(workrave-core-next-integration-test
    ActivityMonitorStub.cc
    IntegrationTests.cc
    SimulatedTime.cc
    )
  target_code_coverage(workrave-core-next-integration-test AUTO EXCLUDE LocalActivityMonitor.cc XmlConfigurator.* GSettingsConfigurator.* usr/include/*)

  set_target_properties(workrave-core-next-integration-test PROPERTIES USE_STUBS ON)

  if (HAVE_APP_GTK)
    target_link_directories(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-core-next)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-config)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-dbus-stub)
  target_link_libraries(workrave-core-next-integration-test PRIVATE Boost::unit_test_framework)
  target_link_libraries(workrave-core-next-integration-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-next-integration-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/corenext/src)

  if (HAVE_APP_QT)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${Qt6DBus_LIBRARIES})
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${Qt6Widgets_LIBRARIES})
  elseif (HAVE_APP_GTK)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARIES})
    target_link_directories(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  if (PLATFORM_OS_UNIX)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
  endif()

  if (SSP_LIBRARY)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${SSP_LIBRARY})
    target_link_libraries(workrave-core-next-timer-test PRIVATE ${SSP_LIBRARY})
  endif()

  workrave_add_test(workrave-core-next-integration-test)
  workrave_add_test(workrave-core-next-timer-test)
endif()
