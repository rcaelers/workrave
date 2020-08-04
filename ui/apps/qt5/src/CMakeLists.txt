set(SRC
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

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    Dock.mm
    MouseMonitor.cc
    )
endif()

set(DESIGNER_UI
)

qt5_wrap_ui(HEADERS_UI ${DESIGNER_UI})
qt5_wrap_cpp(MOC_SRC
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

if (PLATFORM_OS_OSX)
  qt5_wrap_cpp(MOC_SRC
    Dock.hh
    )
endif()

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR} DBusGUI)
  set(SRC ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in ${BINDIR})
endif (HAVE_DBUS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Svg_DEFINITIONS})

add_executable(Workrave MACOSX_BUNDLE ${SRC} ${HEADERS_UI} ${MOC_SRC})
add_cppcheck(Workrave)

target_include_directories(Workrave
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/apps/qt5/src
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
  )

link_directories(${Boost_LIBRARIES_DIR})

target_link_libraries(Workrave
  workrave-ui-common
  workrave-libs-audio
  workrave-libs-config
  workrave-libs-core
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  workrave-libs-updater
  workrave-libs-utils
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5DBus_LIBRARIES}
  ${Qt5MacExtras_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIBINTL_LIBRARIES})

if (HAVE_GLIB)
  target_include_directories(Workrave PRIVATE ${GLIB_INCLUDE_DIRS})
  target_link_directories(Workrave PRIVATE ${GLIB_LIBPATH})
  target_link_libraries(Workrave ${GLIB_LIBRARIES})
endif()

if (APPLE)
  set_target_properties(Workrave PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist
    MACOSX_BUNDLE_BUNDLE_NAME "Workrave"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
    MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")

  set_source_files_properties(BreakWindow.cc PreludeWindow.cc MouseMonitor.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

if (PLATFORM_OS_UNIX)
  target_link_libraries(Workrave ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

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
     target_link_libraries(Workrave harpoon)
     target_include_directories(Workrave PRIVATE ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
  endif()

  target_include_directories(Workrave PRIVATE ${CMAKE_SOURCE_DIR}/ui/applets/win32/include)
  target_link_libraries(Workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)
  target_link_directories(Workrave ${Boost_LIBRARY_DIR})
endif()

configure_file(project.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/Workrave.vcxproj.user @ONLY)

install(TARGETS Workrave RUNTIME DESTINATION bin BUNDLE DESTINATION ".")

# TODO: win32 resource.rc
