set(SRC ${SRC}
  TimerTests.cc
  )

add_executable(workrave-test ${SRC})

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/src
  ${CMAKE_SOURCE_DIR}/backend/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  )

target_link_libraries(workrave-test workrave-backend)
target_link_libraries(workrave-test workrave-libs-dbus)
target_link_libraries(workrave-test workrave-libs-config)
target_link_libraries(workrave-test workrave-libs-utils)

target_link_libraries(workrave-test workrave-libs-input-monitor-stub)

target_link_libraries(workrave-test ${Boost_LIBRARIES})
target_link_libraries(workrave-test ${Qt5DBus_LIBRARIES})
target_link_libraries(workrave-test ${Qt5Widgets_LIBRARIES})


if (PLATFORM_OS_UNIX)
  target_link_libraries(workrave-test ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
endif()

