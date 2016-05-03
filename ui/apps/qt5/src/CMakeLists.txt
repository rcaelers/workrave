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
  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
    )

dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in)

endif (HAVE_DBUS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Svg_DEFINITIONS})

include_directories(
  ${CMAKE_SOURCE_DIR}/ui/apps/qt5/src
  ${CMAKE_SOURCE_DIR}/ui/common/include
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include

  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  ${CMAKE_SOURCE_DIR}/libs/updater/include
  ${CMAKE_SOURCE_DIR}/libs/session/include

  ${CMAKE_SOURCE_DIR}/libs/core/include

  #${INDICATOR_INCLUDE_DIRS}
  )

include_directories()

if (PLATFORM_OS_WIN32)
    link_directories("${HOOK_BINARY_DIR}")
  if (CMAKE_CROSSCOMPILING OR MSYS)
    link_directories("${SYS_ROOT}")
  endif()
endif()

add_executable(Workrave MACOSX_BUNDLE ${SRC} ${HEADERS_UI} ${MOC_SRC})
add_cppcheck(Workrave)
configure_file(project.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/Workrave.vcxproj.user @ONLY)

if (APPLE)
  set_target_properties(Workrave PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/Info.plist
    MACOSX_BUNDLE_BUNDLE_NAME "Workrave"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION}
    MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")

  set_source_files_properties(BreakWindow.cc PreludeWindow.cc MouseMonitor.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")

  ## TODO: set_target_properties(Workrave PROPERTIES LINK_FLAGS "-Wl,-rpath,/${CMAKE_BINARY_DIR}/libs/updater/ext/sparkle/Release/")
endif()

if (PLATFORM_OS_WIN32_NATIVE)
   set_target_properties(Workrave PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

target_link_libraries(Workrave workrave-ui-common)
target_link_libraries(Workrave workrave-libs-core)
target_link_libraries(Workrave workrave-libs-config)
target_link_libraries(Workrave workrave-libs-input-monitor)
target_link_libraries(Workrave workrave-libs-audio)
target_link_libraries(Workrave workrave-libs-dbus)
target_link_libraries(Workrave workrave-libs-session)
target_link_libraries(Workrave workrave-libs-utils)
# TODO: target_link_libraries(Workrave workrave-libs-updater)

link_directories(${Boost_LIBRARIES_DIR})

link_directories(${GLIB_LIBPATH})
target_link_libraries(Workrave ${GLIB_LIBRARIES})

target_link_libraries(Workrave ${Qt5Widgets_LIBRARIES})
target_link_libraries(Workrave ${Qt5Svg_LIBRARIES})
target_link_libraries(Workrave ${Qt5DBus_LIBRARIES})
target_link_libraries(Workrave ${Qt5MacExtras_LIBRARIES})
target_link_libraries(Workrave ${Boost_LIBRARIES})
target_link_libraries(Workrave ${LIBINTL_LIBRARIES})

#qt5_use_modules(helloworld Widgets Sql Network)

if (PLATFORM_OS_UNIX)
  target_link_libraries(Workrave ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

#  add_definitions(-DGNOMELOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")
#
# Desktop file
intltool_merge_desktop(
  ${CMAKE_CURRENT_SOURCE_DIR}/workrave.desktop.in
  ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)
add_custom_target(generate_desktop_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop DESTINATION ${DATADIR}/applications)

endif()

if (PLATFORM_OS_WIN32)
  target_link_libraries(Workrave ${LIBINTL_LIBRARIES})
  if (HAVE_HARPOON)
     target_link_libraries(Workrave harpoon)
  endif()
  target_link_libraries(Workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)

  include_directories(
    ${CMAKE_SOURCE_DIR}/ui/applets/win32/include
    ${CMAKE_SOURCE_DIR}/libs/input-monitor/include)

        if (HAVE_HARPOON)
      include_directories(
        ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
        endif()

   link_directories(${Boost_LIBRARY_DIR})
endif()

install(TARGETS Workrave RUNTIME DESTINATION bin BUNDLE DESTINATION ".")

# TODO: win32 resource.rc
