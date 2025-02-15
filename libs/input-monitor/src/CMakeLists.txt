add_library(workrave-libs-input-monitor STATIC
  InputMonitor.cc
  InputMonitorFactory.cc)

if (PLATFORM_OS_UNIX)
  target_sources(workrave-libs-input-monitor PRIVATE
    unix/RecordInputMonitor.cc
    unix/X11InputMonitor.cc
    unix/XScreenSaverMonitor.cc
    unix/UnixInputMonitorFactory.cc
    unix/MutterInputMonitor.cc
    )

  target_include_directories(workrave-libs-input-monitor PRIVATE ${CMAKE_SOURCE_DIR}/libs/input-monitor/src/unix)

  if (HAVE_GTK)
    target_include_directories(workrave-libs-input-monitor PRIVATE ${GTK_INCLUDE_DIRS})
    target_link_libraries(workrave-libs-input-monitor ${GTK_LIBRARIES})
    target_link_libraries(workrave-libs-input-monitor ${X11_X11_LIB} ${X11_Xtst_LIB} ${X11_Xss_LIB})
  endif()

  if (HAVE_WAYLAND)
    if (HAVE_APP_GTK)
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ext-idle-notify-v1.c ${CMAKE_CURRENT_BINARY_DIR}/ext-idle-notify-v1-client.h
        COMMAND ${WAYLAND_SCANNER} private-code ${CMAKE_CURRENT_SOURCE_DIR}/unix/protocols/ext-idle-notify-v1.xml ${CMAKE_CURRENT_BINARY_DIR}/ext-idle-notify-v1.c
        COMMAND ${WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/unix/protocols/ext-idle-notify-v1.xml ${CMAKE_CURRENT_BINARY_DIR}/ext-idle-notify-v1-client.h
        DEPENDS unix/protocols/ext-idle-notify-v1.xml
        VERBATIM
      )
      target_sources(workrave-libs-input-monitor PRIVATE
        unix/WaylandInputMonitor.cc
        ${CMAKE_CURRENT_BINARY_DIR}/ext-idle-notify-v1.c
        )
      target_link_libraries(workrave-libs-input-monitor ${WAYLAND_CLIENT_LIBRARIES})
    elseif(HAVE_APP_QT)
      qt_generate_wayland_protocol_client_sources(workrave-libs-input-monitor
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/unix/protocols/ext-idle-notify-v1.xml
        PRIVATE_CODE)
        target_sources(workrave-libs-input-monitor PRIVATE unix/QtWaylandInputMonitor.cc)
      target_link_libraries(workrave-libs-input-monitor Qt::WaylandClient Wayland::Client)
    endif()
  endif()

  target_include_directories(workrave-libs-input-monitor PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-input-monitor PRIVATE
    windows/W32AlternateMonitor.cc
    windows/W32InputMonitorFactory.cc
    windows/W32LowLevelMonitor.cc)

  if (HAVE_HARPOON)
    target_sources(workrave-libs-input-monitor PRIVATE
      windows/W32InputMonitor.cc)

    add_library(workrave-libs-input-monitor-harpoon STATIC windows/Harpoon.cc)
    target_include_directories(workrave-libs-input-monitor-harpoon
      PRIVATE
      ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
      ${CMAKE_SOURCE_DIR}/libs/hooks/harpoonHelper/include
      PUBLIC
      ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include
      ${CMAKE_SOURCE_DIR}/libs/input-monitor/include)

    target_link_libraries(workrave-libs-input-monitor-harpoon workrave-libs-utils workrave-libs-config harpoon64)
    target_link_libraries(workrave-libs-input-monitor workrave-libs-input-monitor-harpoon)
  endif()

  target_include_directories(workrave-libs-input-monitor PRIVATE ${CMAKE_SOURCE_DIR}/libs/input-monitor/src/windows)
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
  target_link_libraries(workrave-libs-input-monitor ${GLIB_LIBRARIES})
endif()

target_link_libraries(workrave-libs-input-monitor workrave-libs-utils workrave-libs-config)
if (HAVE_CRASH_REPORT)
  target_link_libraries(workrave-libs-input-monitor workrave-libs-crash)
endif()

target_include_directories(workrave-libs-input-monitor
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  )

add_library(workrave-libs-input-monitor-stub STATIC InputMonitorFactoryStub.cc)

target_include_directories(workrave-libs-input-monitor-stub
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/src
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include)

target_link_libraries(workrave-libs-input-monitor-stub workrave-libs-utils workrave-libs-config)
