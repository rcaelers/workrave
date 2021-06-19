add_executable(Workrave MACOSX_BUNDLE
  AboutDialog.cc
  Application.cc
  BreakWindow.cc
  DailyLimitWindow.cc
  DataConnector.cc
  ExercisesDialog.cc
  ExercisesPanel.cc
  Frame.cc
  GeneralUiPreferencesPanel.cc
  IconListNotebook.cc
  MainWindow.cc
  MenuModel.cc
  Menus.cc
  MicroBreakWindow.cc
  PreferencesDialog.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  SizeGroup.cc
  SoundsPreferencesPanel.cc
  StatisticsDialog.cc
  StatusIcon.cc
  Text.cc
  TimeBar.cc
  TimeEntry.cc
  TimerBoxPreferencesPanel.cc
  TimerBoxView.cc
  TimerPreferencesPanel.cc
  Toolkit.cc
  ToolkitMenu.cc
  Ui.cc
  UiUtil.cc
  main.cc
  qformat.cc
  )

if (PLATFORM_OS_MACOS)
  target_sources(Workrave PRIVATE
    Dock.mm
    MouseMonitor.cc
    ToolkitPlatformMac.cc)
endif()

if (PLATFORM_OS_UNIX)
  target_sources(Workrave PRIVATE ToolkitPlatformLinux.cc)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(Workrave PRIVATE ToolkitPlatformWindows.cc)
endif()

set(MOC_SOURCES
  AboutDialog.hh
  BreakWindow.hh
  DailyLimitWindow.hh
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

if (PLATFORM_OS_MACOS)
  set(MOC_SOURCES ${MOC_SOURCES} Dock.hh)
endif()

qt_wrap_cpp(MOC_SRC ${MOC_SOURCES})

target_sources(Workrave PRIVATE ${MOC_SRC})

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR} DBusGUI)
  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in ${BINDIR})
  target_sources(Workrave PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
endif (HAVE_DBUS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

target_include_directories(Workrave
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/apps/qt5/src
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
  )

target_link_directories(Workrave PRIVATE ${Boost_LIBRARIES_DIR})

target_link_libraries(Workrave PRIVATE
  workrave-ui-common
  workrave-libs-audio
  workrave-libs-config
  workrave-libs-core-next
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  workrave-libs-updater
  workrave-libs-utils
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Svg
  ${Boost_LIBRARIES}
  ${LIBINTL_LIBRARIES})

if (HAVE_QT5 AND PLATFORM_OS_MACOS)
  target_link_libraries(Workrave PRIVATE Qt${QT_VERSION_MAJOR}::MacExtras)
endif()

if (HAVE_GLIB)
  target_include_directories(Workrave PRIVATE ${GLIB_INCLUDE_DIRS})
  target_link_directories(Workrave PRIVATE ${GLIB_LIBPATH})
  target_link_libraries(Workrave PRIVATE ${GLIB_LIBRARIES})
endif()

if (APPLE)
  set_target_properties(Workrave
    PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist
    MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}"
    MACOSX_BUNDLE_ICON_FILE Icon.icns
    MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_NAME}"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION}"
    )

  macdeployqt(Workrave)
  set_source_files_properties(BreakWindow.cc PreludeWindow.cc MouseMonitor.cc ToolkitPlatformMac.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

if (PLATFORM_OS_UNIX)
  target_link_libraries(Workrave PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

  intltool_merge_desktop(${CMAKE_CURRENT_SOURCE_DIR}/workrave.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)
  add_custom_target(generate_desktop_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop DESTINATION ${DATADIR}/applications)
endif()

if (PLATFORM_OS_WINDOWS)
  if (PLATFORM_OS_WINDOWS_NATIVE)
    set_target_properties(Workrave PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  endif()

  target_link_directories(Workrave "${HOOK_BINARY_DIR}")
  if (CMAKE_CROSSCOMPILING OR MSYS)
    target_link_directories(Workrave "${SYS_ROOT}")
  endif()

  target_link_libraries(Workrave ${LIBINTL_LIBRARIES})
  if (HAVE_HARPOON)
     target_link_libraries(Workrave harpoon64)
     target_include_directories(Workrave PRIVATE ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
  endif()

  target_include_directories(Workrave PRIVATE ${CMAKE_SOURCE_DIR}/ui/applets/win32/include)
  target_link_libraries(Workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)
  target_link_directories(Workrave ${Boost_LIBRARY_DIR})
endif()

configure_file(project.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/Workrave.vcxproj.user @ONLY)

install(TARGETS Workrave RUNTIME DESTINATION bin BUNDLE DESTINATION ".")

# TODO: win32 resource.rc
