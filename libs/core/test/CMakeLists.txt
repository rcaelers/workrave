if (HAVE_TESTS)

  set(SRC ${SRC}
    ActivityMonitorStub.cc
    TimerTests.cc
    IntegrationTests.cc
    SimulatedTime.cc
    )

  if (HAVE_APP_GTK)
    link_directories(${GLIB_LIBRARY_DIRS})
  endif()

  add_executable(workrave-test ${SRC})
  add_cppcheck(workrave-test)

  include_directories(
    ${CMAKE_SOURCE_DIR}/libs/core/src
    ${CMAKE_SOURCE_DIR}/libs/core/include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    ${CMAKE_SOURCE_DIR}/libs/config/include
    ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
    ${CMAKE_SOURCE_DIR}/libs/dbus/include
    )

  target_link_libraries(workrave-test workrave-libs-core)
  target_link_libraries(workrave-test workrave-libs-config)
  target_link_libraries(workrave-test workrave-libs-utils)

  target_link_libraries(workrave-test workrave-libs-dbus-stub)
  target_link_libraries(workrave-test workrave-libs-input-monitor-stub)

  target_link_libraries(workrave-test ${Boost_LIBRARIES})

  if (HAVE_APP_QT5)
    target_link_libraries(workrave-test ${Qt5DBus_LIBRARIES})
    target_link_libraries(workrave-test ${Qt5Widgets_LIBRARIES})
  elseif (HAVE_APP_GTK)
    target_link_libraries(workrave-test ${GLIB_LIBRARIES})
  endif()

  if (PLATFORM_OS_UNIX)
    target_link_libraries(workrave-test ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
  endif()

endif()
