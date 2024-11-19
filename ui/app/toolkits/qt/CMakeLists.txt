add_library(workrave-toolkit-qt OBJECT)

target_sources(workrave-toolkit-qt PRIVATE
  AboutDialog.cc
  BreakWindow.cc
  DailyLimitWindow.cc
  DebugDialog.cc
  ExercisesDialog.cc
  ExercisesPanel.cc
  MainWindow.cc
  MicroBreakWindow.cc
  PreferencesDialog.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  StatisticsDialog.cc
  Toolkit.cc
  ToolkitFactory.cc
  ToolkitMenu.cc
  preferences/GeneralUiPreferencesPanel.cc
  preferences/SoundsPreferencesPanel.cc
  preferences/TimerBoxPreferencesPanel.cc
  preferences/TimerPreferencesPanel.cc
  utils/DataConnector.cc
  utils/SizeGroup.cc
  utils/Ui.cc
  utils/UiUtil.cc
  utils/qformat.cc
  widgets/Frame.cc
  widgets/Icon.cc
  widgets/IconListNotebook.cc
  widgets/StatusIcon.cc
  widgets/TimeBar.cc
  widgets/TimeEntry.cc
  widgets/TimerBoxView.cc
)

if (PLATFORM_OS_UNIX)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitLinux.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitWindows.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows)
  target_link_libraries(workrave-toolkit-qt PUBLIC Unfold::unfold cmark webview2 ws2_32 mswsock)
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-toolkit-qt PRIVATE
    ToolkitMacOS.cc
    platforms/macos/MouseMonitor.cc
    platforms/macos/MacOSDesktopWindow.cc)

    set_source_files_properties(BreakWindow.cc PreludeWindow.cc ToolkitMacOS.cc platforms/macos/MouseMonitor.cc platforms/macos/MacOSDesktopWindow.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

set(MOC_SOURCES
  AboutDialog.hh
  AutoUpdateDialog.cc
  BreakWindow.hh
  DailyLimitWindow.hh
  DebugDialog.hh
  ExercisesDialog.hh
  ExercisesPanel.hh
  MainWindow.hh
  MicroBreakWindow.hh
  PreferencesDialog.hh
  PreludeWindow.hh
  RestBreakWindow.hh
  StatisticsDialog.hh
  Toolkit.hh
  ToolkitMenu.hh
  preferences/GeneralUiPreferencesPanel.hh
  preferences/SoundsPreferencesPanel.hh
  preferences/TimerBoxPreferencesPanel.hh
  preferences/TimerPreferencesPanel.hh
  utils/SizeGroup.hh
  widgets/Frame.hh
  widgets/IconListNotebook.hh
  widgets/StatusIcon.hh
  widgets/TimeBar.hh
  widgets/TimeEntry.hh
  widgets/TimerBoxView.hh
)

qt_wrap_cpp(MOC_SRC ${MOC_SOURCES})

target_sources(workrave-toolkit-qt PRIVATE ${MOC_SRC})

target_include_directories(workrave-toolkit-qt
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/utils
  ${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/widgets
  ${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/preferences
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
  )

target_link_directories(workrave-toolkit-qt PRIVATE
  ${Boost_LIBRARIES_DIRS}
  ${X11_X11_INCLUDE_PATH}
  ${X11_INCLUDE_PATH}
  ${X11_INCLUDE_PATH})

if (HAVE_UNFOLD_AUTO_UPDATE)
  # if (PLATFORM_OS_WINDOWS)
  #   target_sources(workrave-toolkit-qt PRIVATE platforms/windows/Edge.cc)
  #   target_link_libraries(workrave-toolkit-qt PUBLIC Unfold::unfold cmark webview2 ws2_32 mswsock)
  #   target_compile_definitions(workrave-toolkit-qt PRIVATE CMARK_NO_SHORT_NAMES)
  # endif()

  target_sources(
    workrave-toolkit-qt
    PRIVATE
    AutoUpdater.cc
    AutoUpdateDialog.cc)
endif()

target_link_libraries(workrave-toolkit-qt PRIVATE
  workrave-app
  workrave-libs-audio
  workrave-libs-config
  workrave-libs-core-next
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  workrave-libs-utils
  workrave-libs-ui-prefwidgets
  workrave-libs-ui-prefwidgets-qt
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Svg
  Qt${QT_VERSION_MAJOR}::Xml
  ${LIBINTL_LIBRARIES}
  ${EXTRA_LIBRARIES}
  ${X11_X11_LIB}
  ${X11_Xtst_LIB}
  ${X11_Xss_LIB}
  fmt::fmt)

if (HAVE_UNFOLD_AUTO_UPDATE)
  target_link_libraries(workrave-toolkit-qt PRIVATE workrave-libs-updater)
endif()

if (HAVE_GLIB)
  target_include_directories(workrave-toolkit-qt PRIVATE ${GLIB_INCLUDE_DIRS})
  target_link_directories(workrave-toolkit-qt PRIVATE ${GLIB_LIBPATH})
  target_link_libraries(workrave-toolkit-qt PRIVATE ${GLIB_LIBRARIES})
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
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Xml
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
