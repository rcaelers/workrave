add_library(workrave-libs-input-monitor STATIC 
  InputMonitor.cc
  InputMonitorFactory.cc)

if (PLATFORM_OS_UNIX)
  target_sources(workrave-libs-input-monitor PRIVATE
    unix/RecordInputMonitor.cc
    unix/X11InputMonitor.cc
    unix/XScreenSaverMonitor.cc
    unix/UnixInputMonitorFactory.cc
    unix/MutterInputMonitor.cc)

  target_include_directories(workrave-libs-input-monitor PRIVATE ${CMAKE_SOURCE_DIR}/libs/input-monitor/src/unix)
  if (HAVE_GTK)
    target_include_directories(workrave-libs-input-monitor PRIVATE ${GTK_INCLUDE_DIRS})
  endif()
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-input-monitor PRIVATE
    win32/W32AlternateMonitor.cc
    win32/W32InputMonitorFactory.cc
    win32/W32LowLevelMonitor.cc)

  if (HAVE_HARPOON)
    target_sources(workrave-libs-input-monitor PRIVATE
      win32/W32InputMonitor.cc
      win32/Harpoon.cc)
    target_include_directories(workrave-libs-input-monitor PRIVATE 
      ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include
      ${CMAKE_SOURCE_DIR}/libs/hooks/harpoonHelper/include)
  endif()

  target_include_directories(workrave-libs-input-monitor PRIVATE ${CMAKE_SOURCE_DIR}/libs/input-monitor/src/win32)
  target_link_libraries(workrave-libs-input-monitor harpoon64)
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-libs-input-monitor PRIVATE
    macos/MacOSInputMonitor.cc
    macos/MacOSInputMonitorFactory.cc)

  target_include_directories(workrave-libs-input-monitor PRIVATE ${CMAKE_SOURCE_DIR}/libs/input-monitor/src/macos)
  target_link_libraries(workrave-libs-input-monitor ${IOKIT_LIBRARY})
endif()

if (HAVE_GLIB)
  target_include_directories(workrave-libs-input-monitor PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

target_link_libraries(workrave-libs-input-monitor workrave-libs-utils workrave-libs-config)

target_include_directories(workrave-libs-input-monitor
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  )

#add_library(workrave-libs-input-monitor-stub STATIC InputMonitorFactoryStub.cc)
#
#target_include_directories(workrave-libs-input-monitor-stub
#  PRIVATE
#  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
#  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
#  )
#
#target_link_libraries(workrave-libs-input-monitor-stub workrave-libs-utils workrave-libs-config)
