add_library(workrave-toolkit-qt OBJECT
  AboutDialog.cc
  BreakWindow.cc
  DailyLimitWindow.cc
  DebugDialog.cc
  ExercisesDialog.cc
  ExercisesPanel.cc
  MainWindow.cc
  MicroBreakWindow.cc
  preferences/GeneralUiPreferencesPanel.cc
  preferences/SoundsPreferencesPanel.cc
  preferences/TimerBoxPreferencesPanel.cc
  preferences/TimerPreferencesPanel.cc
  PreferencesDialog.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  StatisticsDialog.cc
  Toolkit.cc
  ToolkitFactory.cc
  ToolkitMenu.cc
  utils/DataConnector.cc
  utils/qformat.cc
  utils/SizeGroup.cc
  utils/Ui.cc
  utils/UiUtil.cc
  widgets/Frame.cc
  widgets/IconListNotebook.cc
  widgets/StatusIcon.cc
  widgets/TimeBar.cc
  widgets/TimeEntry.cc
  widgets/TimerBoxView.cc
  )

if (PLATFORM_OS_MACOS)
  target_sources(workrave-toolkit-qt PRIVATE
    ToolkitMacOS.cc
    platforms/macos/MouseMonitor.cc
    platforms/macos/MacOSDesktopWindow.cc)

    set_source_files_properties(BreakWindow.cc PreludeWindow.cc ToolkitMacOS.cc platforms/macos/MouseMonitor.cc platforms/macos/MacOSDesktopWindow.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

if (PLATFORM_OS_UNIX)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitLinux.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitWindows.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows)
  target_link_libraries(workrave-toolkit-qt PUBLIC Unfold::unfold cmark::cmark webview2 ws2_32 mswsock)
endif()

set(MOC_SOURCES
  AboutDialog.hh
  BreakWindow.hh
  DailyLimitWindow.hh
  DebugDialog.hh
  ExercisesDialog.hh
  ExercisesPanel.hh
  MainWindow.hh
  MicroBreakWindow.hh
  preferences/GeneralUiPreferencesPanel.hh
  preferences/SoundsPreferencesPanel.hh
  preferences/TimerBoxPreferencesPanel.hh
  preferences/TimerPreferencesPanel.hh
  PreferencesDialog.hh
  PreludeWindow.hh
  RestBreakWindow.hh
  StatisticsDialog.hh
  Toolkit.hh
  ToolkitMenu.hh
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
  ${LIBINTL_LIBRARIES}
  ${EXTRA_LIBRARIES}
  ${X11_X11_LIB}
  ${X11_Xtst_LIB}
  ${X11_Xss_LIB})

if (HAVE_GLIB)
  target_include_directories(workrave-toolkit-qt PRIVATE ${GLIB_INCLUDE_DIRS})
  target_link_directories(workrave-toolkit-qt PRIVATE ${GLIB_LIBPATH})
  target_link_libraries(workrave-toolkit-qt PRIVATE ${GLIB_LIBRARIES})
endif()
