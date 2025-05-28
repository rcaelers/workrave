add_library(workrave-toolkit-gtkmm OBJECT)

target_sources(workrave-toolkit-gtkmm PRIVATE
  BreakWindow.cc
  DBusPreludeWindow.cc
  DailyLimitWindow.cc
  DataConnector.cc
  DebugDialog.cc
  ExercisesDialog.cc
  ExercisesPanel.cc
  GnomeSession.cc
  GtkUtil.cc
  HeadInfo.cc
  Hig.cc
  MainWindow.cc
  MicroBreakWindow.cc
  PreferencesDialog.cc
  Prelude.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  StatisticsDialog.cc
  Toolkit.cc
  ToolkitFactory.cc
  ToolkitMenu.cc
  Ui.cc
  preferences/TimerBoxPreferencePanel.cc
  preferences/TimerPreferencePanel.cc
  preferences/GeneralPreferencePanel.cc
  preferences/SoundPreferencePanel.cc
  preferences/MonitoringPreferencePanel.cc
  widgets/EventButton.cc
  widgets/EventImage.cc
  widgets/EventLabel.cc
  widgets/Frame.cc
  widgets/PanelList.cc
  widgets/TimeBar.cc
  widgets/TimeEntry.cc
  widgets/TimerBoxGtkView.cc
  )

if (HAVE_STATUSICON)
  target_sources(workrave-toolkit-gtkmm PRIVATE widgets/StatusIcon.cc)
endif()

if (PLATFORM_OS_UNIX)
  target_sources(workrave-toolkit-gtkmm PRIVATE
    ToolkitUnix.cc
    platforms/unix/X11SystrayAppletWindow.cc
    platforms/unix/desktop-window.c
    platforms/unix/gtktrayicon.c
    platforms/unix/UnixLocker.cc
    LoginSession.cc
  )

    if (HAVE_WAYLAND)
      add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1.c  ${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1-client.h
        COMMAND ${WAYLAND_SCANNER} private-code ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix/protocols/wlr-layer-shell-unstable-v1.xml ${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1.c
        COMMAND ${WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix/protocols/wlr-layer-shell-unstable-v1.xml ${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1-client.h
        DEPENDS platforms/unix/protocols/wlr-layer-shell-unstable-v1.xml
        VERBATIM
      )

      add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell.c  ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client.h
        COMMAND ${WAYLAND_SCANNER} private-code ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix/protocols/xdg-shell.xml ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell.c
        COMMAND ${WAYLAND_SCANNER} client-header ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix/protocols/xdg-shell.xml ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client.h
        DEPENDS platforms/unix/protocols/xdg-shell.xml
        VERBATIM
      )

      target_sources(workrave-toolkit-gtkmm PRIVATE
        platforms/unix/WaylandWindowManager.cc
        ${CMAKE_CURRENT_BINARY_DIR}/wlr-layer-shell-unstable-v1.c
        ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell.c)
    endif()

    if (HAVE_DBUSMENU)
      target_sources(workrave-toolkit-gtkmm PRIVATE platforms/unix/DbusMenu.cc)
    endif()

    if (HAVE_APPINDICATOR)
      target_sources(workrave-toolkit-gtkmm PRIVATE platforms/unix/AppIndicatorMenu.cc)
    endif()

    target_include_directories(workrave-toolkit-gtkmm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix/protocols)


    target_include_directories(workrave-toolkit-gtkmm PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/platforms/unix/)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-toolkit-gtkmm PRIVATE
    ToolkitWindows.cc
    # ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
  )

  target_include_directories(workrave-toolkit-gtkmm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows)
  target_link_libraries(workrave-toolkit-gtkmm PUBLIC ${LIBINTL_LIBRARIES})
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-toolkit-gtkmm PRIVATE
    ToolkitMacOS.cc
  #   platforms/macos/MacOSAppletWindow.cc
  #   platforms/macos/MacOSGtkMenu.cc
  #   platforms/macos/MacOSUtil.cc
  #   platforms/macos/MacOSStatusBarView.mm
  #   platforms/macos/MacOSTimeBar.mm
  )

  # set_source_files_properties(macos/MacOSAppletWindow.cc macos/MacOSGtkMenu.cc macos/MacOSUtil.cc Locale.cc AppletControl.cc macos/MacOSTimeBar.mm macos/MacOSStatusBarView.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")

  target_include_directories(workrave-toolkit-gtkmm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

target_include_directories(
  workrave-toolkit-gtkmm
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/preferences
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
  ${GTK_INCLUDE_DIRS}
  ${DBUSMENU_INCLUDE_DIRS}
  ${INDICATOR_INCLUDE_DIRS}
  ${APPINDICATOR_INCLUDE_DIRS}
  )

target_compile_definitions(workrave-toolkit-gtkmm PRIVATE GDK_DISABLE_DEPRECATED GTK_DISABLE_DEPRECATED)

target_link_directories(workrave-toolkit-gtkmm PRIVATE ${GTK_LIBRARY_DIRS})
target_link_directories(workrave-toolkit-gtkmm PRIVATE ${INDICATOR_LIBPATH})
target_link_directories(workrave-toolkit-gtkmm PRIVATE ${fAPPINDICATOR_LIBPATH})
target_link_directories(workrave-toolkit-gtkmm PRIVATE ${DBUSMENU_LIBPATH})

If (PLATFORM_OS_WINDOWS)
  if (CMAKE_CROSSCOMPILING)
    target_link_directories(workrave-toolkit-gtkmm PRIVATE "${SYS_ROOT}")
  endif()
endif()

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

execute_process(
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${GLIB_COMPILE_RESOURCES} --sourcedir=${CMAKE_SOURCE_DIR}/ui/data --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-dependencies resources.gresource.xml
  OUTPUT_VARIABLE RESOURCES_DEPENDENCIES
)
string( REPLACE "\n" ";" RESOURCES_DEPENDENCIES ${RESOURCES_DEPENDENCIES} )

add_custom_command(
  OUTPUT resources.gresource.xml.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${GLIB_COMPILE_RESOURCES}
  ARGS
    --sourcedir=${CMAKE_SOURCE_DIR}/ui/data
    --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}
    --target=${CMAKE_CURRENT_BINARY_DIR}/resources.gresource.xml.c
    --generate-source
    ${CMAKE_CURRENT_SOURCE_DIR}/resources.gresource.xml
  VERBATIM
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/resources.gresource.xml
  DEPENDS ${RESOURCES_DEPENDENCIES}
)

target_sources(
  workrave-toolkit-gtkmm
  PRIVATE
  resources.gresource.xml.c)

if (HAVE_UNFOLD_AUTO_UPDATE)
  if (PLATFORM_OS_WINDOWS)
    target_sources(workrave-toolkit-gtkmm PRIVATE platforms/windows/Edge.cc)
    target_link_libraries(workrave-toolkit-gtkmm PUBLIC Unfold::unfold cmark webview2 ws2_32 mswsock)
    target_compile_definitions(workrave-toolkit-gtkmm PRIVATE CMARK_NO_SHORT_NAMES)
  endif()

  target_sources(
    workrave-toolkit-gtkmm
    PRIVATE
    AutoUpdater.cc
    AutoUpdateDialog.cc)
endif()

target_link_libraries(workrave-toolkit-gtkmm
  PUBLIC
  PRIVATE
  workrave-app
  workrave-libs-audio
  workrave-libs-config
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  workrave-libs-utils
  workrave-libs-ui-prefwidgets
  workrave-libs-ui-prefwidgets-gtkmm
  fmt::fmt)

if (HAVE_UNFOLD_AUTO_UPDATE)
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE workrave-libs-updater)
endif()

if (HAVE_CORE_NEXT)
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE workrave-libs-core-next)
else()
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE workrave-libs-core)
endif()

target_link_libraries(workrave-toolkit-gtkmm PRIVATE ${GTK_LIBRARIES})
target_link_libraries(workrave-toolkit-gtkmm PRIVATE ${EXTRA_LIBRARIES})
target_link_directories(workrave-toolkit-gtkmm PUBLIC ${GTK_LIBRARY_DIRS})

if (PLATFORM_OS_UNIX)
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE ${INDICATOR_LIBRARIES})
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE ${APPINDICATOR_LIBRARIES})
  target_link_libraries(workrave-toolkit-gtkmm PRIVATE ${DBUSMENU_LIBRARIES})
  target_compile_definitions(workrave-toolkit-gtkmm PRIVATE -DGNOMELOCALEDIR="${CMAKE_INSTALL_PREFIX}/${DATADIR}/locale")
endif()

if (HAVE_CRASH_REPORT)
  add_executable(WorkraveCrashHandler CrashDialog.cc)

  set(WR_VERSION ${WORKRAVE_VERSION})
  set(WR_RESOURCE_VERSION ${WORKRAVE_RESOURCE_VERSION})
  set(WR_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)

  target_sources(WorkraveCrashHandler PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)

  target_include_directories(WorkraveCrashHandler PRIVATE ${GTK_INCLUDE_DIRS})
  target_link_directories(WorkraveCrashHandler PRIVATE ${GTK_LIBRARY_DIRS})
  target_include_directories(WorkraveCrashHandler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/commonui/include)

  target_link_libraries(WorkraveCrashHandler
    PRIVATE
    crashpad::handler_lib
    crashpad::tools
    workrave-libs-utils
    ${GTK_LIBRARIES}
    ${LIBINTL_LIBRARIES}
    )

  if (MSVC)
    target_link_options(WorkraveCrashHandler PRIVATE "/SUBSYSTEM:WINDOWS")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(WorkraveCrashHandler PROPERTIES COMPILE_FLAGS "-municode")
    set_target_properties(WorkraveCrashHandler PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows -municode")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(WorkraveCrashHandler PROPERTIES COMPILE_FLAGS "-mwindows -municode")
    set_target_properties(WorkraveCrashHandler PROPERTIES LINK_FLAGS "-mwindows -municode")
  endif()

  install(TARGETS WorkraveCrashHandler RUNTIME DESTINATION "${BINDIR}")
endif()
