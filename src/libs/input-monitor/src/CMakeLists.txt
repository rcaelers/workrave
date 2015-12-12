
set(SRC
  InputMonitor.cc
  InputMonitorFactory.cc
  )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    unix/RecordInputMonitor.cc
    unix/X11InputMonitor.cc
    unix/XScreenSaverMonitor.cc
    unix/UnixInputMonitorFactory.cc)

  include_directories(${CMAKE_SOURCE_DIR}/src/libs/input-monitor/src/unix)

  if (HAVE_GTK)
    include_directories(${GTK_INCLUDE_DIRS})
  endif()
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC}
    win32/W32AlternateMonitor.cc
    win32/W32InputMonitorFactory.cc
    win32/W32LowLevelMonitor.cc)

  if (HAVE_HARPOON)
    set(SRC ${SRC}
      win32/W32InputMonitor.cc
      win32/Harpoon.cc)
    include_directories(
      ${CMAKE_SOURCE_DIR}/src/libs/hooks/harpoon/include
      ${CMAKE_SOURCE_DIR}/src/libs/hooks/harpoonHelper/include)
  endif()

  include_directories(
    ${CMAKE_SOURCE_DIR}/src/libs/input-monitor/src/win32)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    osx/OSXInputMonitor.cc
    osx/OSXInputMonitorFactory.cc)

  include_directories(${CMAKE_SOURCE_DIR}/src/libs/input-monitor/src/osx)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src/libs/input-monitor/src
  ${CMAKE_SOURCE_DIR}/src/libs/input-monitor/include
  ${CMAKE_SOURCE_DIR}/src/libs/utils/include
  ${CMAKE_SOURCE_DIR}/src/libs/config/include
  )

add_library(workrave-libs-input-monitor STATIC ${SRC})
add_cppcheck(workrave-libs-input-monitor)
add_library(workrave-libs-input-monitor-stub STATIC InputMonitorFactoryStub.cc)
add_cppcheck(workrave-libs-input-monitor-stub)

if (PLATFORM_OS_OSX)
  target_link_libraries(workrave-libs-input-monitor ${IOKIT_LIBRARY})
endif()
