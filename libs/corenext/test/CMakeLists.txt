if (HAVE_TESTS)
  add_executable(workrave-core-next-timer-test
    SimulatedTime.cc
    TimerTests.cc)
  target_code_coverage(workrave-core-next-timer-test AUTO)

  target_link_libraries(workrave-core-next-timer-test PRIVATE workrave-libs-core-next)
  target_link_libraries(workrave-core-next-timer-test PRIVATE ${Boost_LIBRARIES})
  target_link_libraries(workrave-core-next-timer-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-next-timer-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/corenext/src)

  add_executable(workrave-core-next-integration-test
    ActivityMonitorStub.cc
    IntegrationTests.cc
    SimulatedTime.cc
    )
  target_code_coverage(workrave-core-next-integration-test AUTO EXCLUDE LocalActivityMonitor.cc XmlConfigurator.* GSettingsConfigurator.* usr/include/*)

  if (HAVE_APP_GTK)
    link_directories(${GLIB_LIBRARY_DIRS})
  endif()


  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-core-next)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-config)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-dbus-stub)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-input-monitor-stub)
  target_link_libraries(workrave-core-next-integration-test PRIVATE ${Boost_LIBRARIES})
  target_link_libraries(workrave-core-next-integration-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-next-integration-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/corenext/src)

  if (HAVE_APP_QT)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${Qt5DBus_LIBRARIES})
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${Qt5Widgets_LIBRARIES})
  elseif (HAVE_APP_GTK)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARIES})
    target_link_directories(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  if (PLATFORM_OS_UNIX)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
  endif()

  add_test(NAME workrave-core-next-integration-test COMMAND workrave-core-next-integration-test)
  add_test(NAME workrave-core-next-timer-test COMMAND workrave-core-next-timer-test)
endif()
