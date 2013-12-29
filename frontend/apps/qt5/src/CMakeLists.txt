set(SRC
  main.cc
  Frame.cc
  MainWindow.cc
  PreludeWindow.cc
  TimerBoxView.cc
  TimeBar.cc
  Toolkit.cc
  BreakWindow.cc
  MicroBreakWindow.cc
  DailyLimitWindow.cc
  RestBreakWindow.cc
  UiUtil.cc
  ExercisesPanel.cc
  ExercisesDialog.cc
  PreferencesDialog.cc
  TimerPreferencesPanel.cc
  TimeEntry.cc
  SizeGroup.cc
  DataConnector.cc
  GeneralUiPreferencesPanel.cc
  SoundsPreferencesPanel.cc
  TimerBoxPreferencesPanel.cc
  MenuHandler.cc
  )

set(DESIGNER_UI
)

qt5_wrap_ui(HEADERS_UI ${DESIGNER_UI})
qt5_wrap_cpp(MOC_SRC
  BreakWindow.hh
  MicroBreakWindow.hh
  DailyLimitWindow.hh
  RestBreakWindow.hh
  MainWindow.hh
  PreludeWindow.hh
  TimeBar.hh
  TimerBoxView.hh
  Toolkit.hh
  Frame.hh
  ExercisesPanel.hh
  ExercisesDialog.hh
  PreferencesDialog.hh
  TimerPreferencesPanel.hh
  TimeEntry.hh
  SizeGroup.hh
  GeneralUiPreferencesPanel.hh
  SoundsPreferencesPanel.hh
  TimerBoxPreferencesPanel.hh
  MenuHandler.hh
)


#if (HAVE_DBUS)
#  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
#  set(SRC ${SRC}
#    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
#    GenericDBusApplet.cc
#    )
#
#  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in)

#endif (HAVE_DBUS)
#

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/apps/qt5/src/
  ${CMAKE_SOURCE_DIR}/frontend/apps/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/common/include/

  ${CMAKE_SOURCE_DIR}/frontend/applets/indicator/include

  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include

  ${CMAKE_SOURCE_DIR}/backend/include/

  #${INDICATOR_INCLUDE_DIRS}
  )

include_directories()

add_executable(workrave-qt ${SRC} ${HEADERS_UI} ${MOC_SRC})

target_link_libraries(workrave-qt workrave-frontend-apps-common)
target_link_libraries(workrave-qt workrave-backend)
target_link_libraries(workrave-qt workrave-libs-config)
target_link_libraries(workrave-qt workrave-libs-input-monitor)
target_link_libraries(workrave-qt workrave-libs-audio)
target_link_libraries(workrave-qt workrave-libs-dbus)
target_link_libraries(workrave-qt workrave-libs-utils)

#link_directories(${GDK_LIBPATH})
#target_link_libraries(workrave-qt ${GDK_LIBRARIES})

target_link_libraries(workrave-qt ${Qt5Widgets_LIBRARIES})
target_link_libraries(workrave-qt ${Qt5DBus_LIBRARIES})
target_link_libraries(workrave-qt ${Boost_LIBRARIES})
target_link_libraries(workrave-qt ${GETTEXT_LIBRARIES})

#qt5_use_modules(helloworld Widgets Sql Network)

if (PLATFORM_OS_UNIX)
  target_link_libraries(workrave-qt ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

#  add_definitions(-DGNOMELOCALEDIR="${DATADIR}/locale")
#
# Desktop file
intltool_merge_desktop(
  ${CMAKE_CURRENT_SOURCE_DIR}/workrave.desktop.in
  ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)
add_custom_target(generate_desktop_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.desktop DESTINATION share/applications)

endif()

#link_directories(${INDICATOR_LIBPATH})
#target_link_libraries(workrave ${INDICATOR_LIBRARIES})
#
#if (WIN32)
#  target_link_libraries(workrave harpoon)
#  target_link_libraries(workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)
#
#  if (HAVE_DIRECTX)
#    link_directories(${DIRECTX_LIBS})
#    target_link_libraries(workrave dxerr dsound dxguid)
#  endif(HAVE_DIRECTX)
#
#  configure_file(
#    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc.in
#    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
#
#endif(WIN32)

install(TARGETS workrave-qt RUNTIME DESTINATION bin)

# TODO: win32 resource.rc
