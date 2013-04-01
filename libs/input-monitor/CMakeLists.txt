
set(SRC
  src/InputMonitor.cc
  src/InputMonitorFactory.cc
  )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    src/unix/RecordInputMonitor.cc
    src/unix/X11InputMonitor.cc
    src/unix/XScreenSaverMonitor.cc
    src/unix/UnixInputMonitorFactory.cc)
  
  include_directories(${CMAKE_SOURCE_DIR}/libs/input-monitor/src/unix)

  if (HAVE_GTK)
    include_directories(${GTK_INCLUDE_DIRS})
  endif()
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC}
    src/win32/W32AlternateMonitor.cc
    src/win32/W32InputMonitor.cc
    src/win32/W32InputMonitorFactory.cc
    src/win32/W32LowLevelMonitor.cc)

  include_directories(${CMAKE_SOURCE_DIR}/libs/input-monitor/src/win32)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    src/osx/OSXInputMonitor.cc
    src/osx/OSXInputMonitorFactory.cc)

  include_directories(${CMAKE_SOURCE_DIR}/libs/input-monitor/src/osx)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  )

add_library(workrave-libs-input-monitor STATIC ${SRC})
