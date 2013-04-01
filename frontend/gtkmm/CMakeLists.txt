set(SRC
  src/AppletControl.cc
  src/AppletWindow.cc
  src/BreakWindow.cc
  src/DailyLimitWindow.cc
  src/DataConnector.cc
  src/EventButton.cc
  src/EventImage.cc
  src/EventLabel.cc
  src/Frame.cc
  src/GUI.cc
  src/GUIConfig.cc
  src/GtkUtil.cc
  src/HeadInfo.cc
  src/Hig.cc
  src/IconListCellRenderer.cc
  src/IconListNotebook.cc
  src/MainGtkMenu.cc
  src/MainWindow.cc
  src/Menus.cc
  src/MicroBreakWindow.cc
  src/PreferencesDialog.cc
  src/PreludeWindow.cc
  src/RestBreakWindow.cc
  src/Session.cc
  src/StatusIcon.cc
  src/TimeBar.cc
  src/TimeEntry.cc
  src/TimerBoxGtkView.cc
  src/TimerBoxPreferencePage.cc
  src/TimerPreferencesPanel.cc
  src/WindowHints.cc
  src/main.cc
  )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    src/unix/GnomeAppletMenu.cc
    src/unix/GnomeAppletWindow.cc
    src/unix/IndicatorAppletMenu.cc
    src/unix/Plug.cc
    src/unix/X11SystrayAppletWindow.cc
    src/unix/desktop-window.c
    src/unix/gtktrayicon.c
    )

  include_directories(${CMAKE_SOURCE_DIR}/frontend/gtkmm/src/unix)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC}
    src/win32/DesktopWindow.cc
    src/win32/W32AppletMenu.cc
    src/win32/W32AppletWindow.cc
    src/win32/W32Compat.cc
    src/win32/W32ForceFocus.cc
    src/win32/W32StatusIcon.cc
    src/win32/W32TrayMenu.cc
    )

  include_directories(${CMAKE_SOURCE_DIR}/frontend/gtkmm/src/win32)
  
  if (HAVE_DIRECTX)
    include_directories(${DIRECTX_INCLUDES})
  endif(HAVE_DIRECTX)
  
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    src/osx/OSXAppletWindow.cc
    src/osx/OSXGtkMenu.cc
    src/osx/OSXUtil.cc
    )

  include_directories(${CMAKE_SOURCE_DIR}/frontend/gtkmm/src/osx)
endif()

if (HAVE_DBUS)
  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
    src/GenericDBusApplet.cc
    )

  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/src/org.workrave.Workrave.service.in)
  
endif (HAVE_DBUS)

if (HAVE_EXERCISES)
  set (EXERCISES_HOME ${CMAKE_SOURCE_DIR}/frontend/plugin/exercises)

  include_directories(
    ${EXERCISES_HOME}/gtkmm/src
    ${EXERCISES_HOME}/common/src
    )

  set(SRC ${SRC}
    ${EXERCISES_HOME}/gtkmm/src/ExercisesDialog.cc
    ${EXERCISES_HOME}/gtkmm/src/ExercisesPanel.cc
    ${EXERCISES_HOME}/common/src/Exercise.cc
    )
endif()

if (HAVE_STATISTICS)
  set (STATISTICS_HOME ${CMAKE_SOURCE_DIR}/frontend/plugin/statistics)

  include_directories(
    ${STATISTICS_HOME}/gtkmm/src
    )
    
  set(SRC ${SRC}
    ${STATISTICS_HOME}/gtkmm/src/StatisticsDialog.cc
    )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/gtkmm/src/
  ${CMAKE_SOURCE_DIR}/frontend/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/applets/indicator/include
  
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include

  ${CMAKE_SOURCE_DIR}/backend/include/
  
  ${GTK_INCLUDE_DIRS}
  ${INDICATOR_INCLUDE_DIRS}
  )

add_executable(workrave ${SRC})

target_link_libraries(workrave workrave-frontend-common)
target_link_libraries(workrave workrave-backend)
target_link_libraries(workrave workrave-libs-config)
target_link_libraries(workrave workrave-libs-input-monitor)
target_link_libraries(workrave workrave-libs-dbus)
target_link_libraries(workrave workrave-libs-utils)

link_directories(${GTK_LIBPATH})
target_link_libraries(workrave ${GTK_LIBRARIES})

if (PLATFORM_OS_UNIX)
  target_link_libraries(workrave ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

  add_definitions(-DGNOMELOCALEDIR="${DATADIR}/locale")

  # Desktop file
  intltool_merge_desktop(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/workrave.desktop)
  add_custom_target(generate_desktop_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/src/workrave.desktop)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/workrave.desktop DESTINATION share/applications)
endif()

link_directories(${INDICATOR_LIBPATH})
target_link_libraries(workrave ${INDICATOR_LIBRARIES})

if (WIN32)
  target_link_libraries(workrave harpoon)
  target_link_libraries(workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)

  if (HAVE_DIRECTX)
    link_directories(${DIRECTX_LIBS})
    target_link_libraries(workrave dxerr dsound dxguid)
  endif(HAVE_DIRECTX)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resource.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/resource.rc)

endif(WIN32)

install(TARGETS workrave RUNTIME DESTINATION bin)

# TODO: win32 resource.rc
