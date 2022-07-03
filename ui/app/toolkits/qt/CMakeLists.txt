add_library(workrave-toolkit-qt OBJECT
  AboutDialog.cc
  BreakWindow.cc
  DailyLimitWindow.cc
  DataConnector.cc
  DebugDialog.cc
  ExercisesDialog.cc
  ExercisesPanel.cc
  Frame.cc
  GeneralUiPreferencesPanel.cc
  IconListNotebook.cc
  MainWindow.cc
  MicroBreakWindow.cc
  PreferencesDialog.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  SizeGroup.cc
  SoundsPreferencesPanel.cc
  StatisticsDialog.cc
  StatusIcon.cc
  TimeBar.cc
  TimeEntry.cc
  TimerBoxPreferencesPanel.cc
  TimerBoxView.cc
  TimerPreferencesPanel.cc
  Toolkit.cc
  ToolkitFactory.cc
  ToolkitMenu.cc
  Ui.cc
  UiUtil.cc
  qformat.cc
  )

if (PLATFORM_OS_MACOS)
  target_sources(workrave-toolkit-qt PRIVATE
    MouseMonitor.cc
    ToolkitMacOS.cc
    platforms/macos/MacOSDesktopWindow.cc)

    set_source_files_properties(BreakWindow.cc PreludeWindow.cc MouseMonitor.cc ToolkitMacOS.cc platforms/macos/MacOSDesktopWindow.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

if (PLATFORM_OS_UNIX)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitLinux.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitWindows.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows)
endif()

set(MOC_SOURCES
  AboutDialog.hh
  BreakWindow.hh
  DailyLimitWindow.hh
  DebugDialog.hh
  ExercisesDialog.hh
  ExercisesPanel.hh
  Frame.hh
  GeneralUiPreferencesPanel.hh
  IconListNotebook.hh
  MainWindow.hh
  MicroBreakWindow.hh
  PreferencesDialog.hh
  PreludeWindow.hh
  RestBreakWindow.hh
  SizeGroup.hh
  SoundsPreferencesPanel.hh
  StatisticsDialog.hh
  StatusIcon.hh
  TimeBar.hh
  TimeEntry.hh
  TimerBoxPreferencesPanel.hh
  TimerBoxView.hh
  TimerPreferencesPanel.hh
  Toolkit.hh
  ToolkitMenu.hh
)

qt_wrap_cpp(MOC_SRC ${MOC_SOURCES})

target_sources(workrave-toolkit-qt PRIVATE ${MOC_SRC})

target_include_directories(workrave-toolkit-qt
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/src
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
  workrave-libs-updater
  workrave-libs-utils
  workrave-libs-ui-prefwidgets
  # workrave-libs-ui-prefwidgets-qt
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
