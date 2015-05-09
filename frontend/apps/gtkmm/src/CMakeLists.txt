set(SRC
  AppletControl.cc
  AppletWindow.cc
  BreakWindow.cc
  DailyLimitWindow.cc
  DataConnector.cc
  EventButton.cc
  EventImage.cc
  EventLabel.cc
  Frame.cc
  GUI.cc
  GtkUtil.cc
  HeadInfo.cc
  Hig.cc
  IconListCellRenderer.cc
  IconListNotebook.cc
  MainGtkMenu.cc
  MainWindow.cc
  Menus.cc
  MicroBreakWindow.cc
  PreferencesDialog.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  StatusIcon.cc
  TimeBar.cc
  TimeEntry.cc
  TimerBoxGtkView.cc
  TimerBoxPreferencePage.cc
  TimerPreferencesPanel.cc
  WindowHints.cc
  main.cc
  )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    unix/GnomeAppletMenu.cc
    unix/GnomeAppletWindow.cc
    unix/Plug.cc
    unix/X11SystrayAppletWindow.cc
    unix/desktop-window.c
    unix/gtktrayicon.c
    )

    if (HAVE_INDICATOR)
      set(SRC ${SRC}
        unix/IndicatorAppletMenu.cc
      )
    endif()
    
  include_directories(${CMAKE_SOURCE_DIR}/frontend/apps/gtkmm/src/unix)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC}
    win32/DesktopWindow.cc
    win32/W32AppletMenu.cc
    win32/W32AppletWindow.cc
    win32/W32Compat.cc
    win32/W32ForceFocus.cc
    win32/W32StatusIcon.cc
    win32/W32TrayMenu.cc
    )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
  
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource_rc.o
                     COMMAND ${CMAKE_RC_COMPILER} -o ${CMAKE_CURRENT_BINARY_DIR}/resource_rc.o
                                                  -i ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	             DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resource.rc			    
                    )
   	
  set(SRC ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/resource_rc.o)

  include_directories(${CMAKE_SOURCE_DIR}/frontend/apps/gtkmm/src/win32)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    osx/OSXAppletWindow.cc
    osx/OSXGtkMenu.cc
    osx/OSXUtil.cc
    )

  include_directories(${CMAKE_SOURCE_DIR}/frontend/apps/gtkmm/src/osx)
endif()

if (HAVE_DBUS)
  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
    GenericDBusApplet.cc
    )

  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in)
  
endif (HAVE_DBUS)

set(SRC ${SRC}
  ExercisesDialog.cc
  ExercisesPanel.cc
  )

set(SRC ${SRC}
  StatisticsDialog.cc
    )

if (HAVE_GSETTINGS)
  gsettings_add_schemas(workrave-backend ${CMAKE_SOURCE_DIR}/frontend/apps/gtkmm/src)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/apps/gtkmm/src/
  ${CMAKE_SOURCE_DIR}/frontend/apps/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/applets/indicator/include
  
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include

  ${CMAKE_SOURCE_DIR}/backend/include/
  
  ${GTK_INCLUDE_DIRS}
  ${INDICATOR_INCLUDE_DIRS}
  )

link_directories(${GTK_LIBRARY_DIRS})
link_directories(${INDICATOR_LIBPATH})

# TODO: Also built prebuilt
If (PLATFORM_OS_WIN32 AND CMAKE_CROSSCOMPILING)
  link_directories("${PREBUILT_PATH}/Release")
  link_directories("${SYS_ROOT}")
endif()

add_executable(workrave ${SRC})
add_cppcheck(workrave)

target_link_libraries(workrave workrave-frontend-common)
target_link_libraries(workrave workrave-backend)
target_link_libraries(workrave workrave-libs-config)
target_link_libraries(workrave workrave-libs-input-monitor)
target_link_libraries(workrave workrave-libs-dbus)
target_link_libraries(workrave workrave-libs-utils)
target_link_libraries(workrave workrave-libs-audio)

target_link_libraries(workrave ${GTK_LIBRARIES})
target_link_libraries(workrave ${Boost_LIBRARIES})

if (PLATFORM_OS_UNIX)
  target_link_libraries(workrave ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

  add_definitions(-DGNOMELOCALEDIR="${DATADIR}/locale")

  # Desktop file
  intltool_merge_desktop(
    ${CMAKE_CURRENT_SOURCE_DIR}/workrave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)
  add_custom_target(generate_desktop_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop DESTINATION ${DATADIR}/applications)
endif()

target_link_libraries(workrave ${INDICATOR_LIBRARIES})

if (WIN32)
  target_link_libraries(workrave harpoon)
  target_link_libraries(workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)

  include_directories(
    ${CMAKE_SOURCE_DIR}/frontend/applets/win32/include
    ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
    ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include
    )

   target_link_libraries(workrave dxerr8 dsound dxguid)
endif(WIN32)

install(TARGETS workrave RUNTIME DESTINATION ${BINDIR})
