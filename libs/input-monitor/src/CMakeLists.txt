set(SRC
  InputMonitor.cc
  InputMonitorFactory.cc
  )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    unix/RecordInputMonitor.cc
    unix/X11InputMonitor.cc
    unix/XScreenSaverMonitor.cc
    unix/UnixInputMonitorFactory.cc
    unix/MutterInputMonitor.cc)

  include_directories(${CMAKE_SOURCE_DIR}/libs/input-monitor/src/unix)

  if (HAVE_GTK)
    include_directories(${GTK_INCLUDE_DIRS})
  endif()
endif()

if (PLATFORM_OS_WINDOWS)
  set(SRC ${SRC}
    win32/W32AlternateMonitor.cc
    win32/W32InputMonitorFactory.cc
    win32/W32LowLevelMonitor.cc)

  if (HAVE_HARPOON)
    set(SRC ${SRC}
      win32/W32InputMonitor.cc
      win32/Harpoon.cc)
    include_directories(
      ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include
      ${CMAKE_SOURCE_DIR}/libs/hooks/harpoonHelper/include)
  endif()

  include_directories(
    ${CMAKE_SOURCE_DIR}/libs/input-monitor/src/win32)
endif()

if (PLATFORM_OS_MACOS)
  set(SRC ${SRC}
    macos/MacOSInputMonitor.cc
    macos/MacOSInputMonitorFactory.cc)

  include_directories(${CMAKE_SOURCE_DIR}/libs/input-monitor/src/macos)
endif()

add_library(workrave-libs-input-monitor STATIC ${SRC})
add_cppcheck(workrave-libs-input-monitor)

if (PLATFORM_OS_MACOS)
  target_link_libraries(workrave-libs-input-monitor ${IOKIT_LIBRARY})
endif()

target_link_libraries(workrave-libs-input-monitor workrave-libs-utils workrave-libs-config)

if (HAVE_GLIB)
  target_include_directories(workrave-libs-input-monitor PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

target_include_directories(workrave-libs-input-monitor
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  )

add_library(workrave-libs-input-monitor-stub STATIC InputMonitorFactoryStub.cc)
add_cppcheck(workrave-libs-input-monitor-stub)

target_include_directories(workrave-libs-input-monitor-stub
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
  )

target_link_libraries(workrave-libs-input-monitor-stub workrave-libs-utils workrave-libs-config)
