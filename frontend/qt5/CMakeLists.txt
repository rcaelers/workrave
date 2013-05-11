set(SRC
  src/main.cc
  src/Frame.cc
  src/Application.cc
  src/MainWindow.cc
  src/PreludeWindow.cc
  src/TimerBoxView.cc
  src/TimeBar.cc
  src/Toolkit.cc
  src/GUIConfig.cc
  src/Session.cc
  src/BreakWindow.cc
  src/MicroBreakWindow.cc
  src/DailyLimitWindow.cc
  src/RestBreakWindow.cc
  src/UiUtil.cc
  src/ExercisesPanel.cc
  src/PreferencesDialog.cc
  src/TimerPreferencesPanel.cc
  src/TimeEntry.cc
  src/SizeGroup.cc
  src/DataConnector.cc
  
  ../plugin/exercises/common/src/Exercise.cc
  
  )

set(DESIGNER_UI
)

qt5_wrap_ui(HEADERS_UI ${DESIGNER_UI})
qt5_wrap_cpp(MOC_SRC
  src/BreakWindow.hh
  src/MicroBreakWindow.hh
  src/DailyLimitWindow.hh
  src/RestBreakWindow.hh
  src/MainWindow.hh
  src/PreludeWindow.hh
  src/TimeBar.hh
  src/TimerBoxView.hh
  src/Toolkit.hh
  src/Frame.hh
  src/ExercisesPanel.hh
  src/PreferencesDialog.hh
  src/TimerPreferencesPanel.hh
  src/TimeEntry.hh
  src/SizeGroup.hh
  src/DataConnector.hh
  )


#if (HAVE_DBUS)
#  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
#  set(SRC ${SRC}
#    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
#    src/GenericDBusApplet.cc
#    )
#
#  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/src/org.workrave.Workrave.service.in)

#endif (HAVE_DBUS)
#
#if (HAVE_EXERCISES)
#  set (EXERCISES_HOME ${CMAKE_SOURCE_DIR}/frontend/plugin/exercises)
#
#  include_directories(
#    ${EXERCISES_HOME}/gtkmm/src
#    ${EXERCISES_HOME}/common/src
#    )
#
#  set(SRC ${SRC}
#    ${EXERCISES_HOME}/gtkmm/src/ExercisesDialog.cc
#    ${EXERCISES_HOME}/gtkmm/src/ExercisesPanel.cc
#    ${EXERCISES_HOME}/common/src/Exercise.cc
#    )
#endif()
#
#if (HAVE_STATISTICS)
#  set (STATISTICS_HOME ${CMAKE_SOURCE_DIR}/frontend/plugin/statistics)
#
#  include_directories(
#    ${STATISTICS_HOME}/gtkmm/src
#    )
#    
#  set(SRC ${SRC}
#    ${STATISTICS_HOME}/gtkmm/src/StatisticsDialog.cc
#    )
#endif()

set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/qt5/src/
  ${CMAKE_SOURCE_DIR}/frontend/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/applets/indicator/include

  ${CMAKE_SOURCE_DIR}/frontend/plugin/exercises/common/src/
  
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include

  ${CMAKE_SOURCE_DIR}/backend/include/

  #${GLIB_INCLUDE_DIRS}
  #${INDICATOR_INCLUDE_DIRS}
  )

include_directories()

add_executable(workrave-qt ${SRC} ${HEADERS_UI} ${MOC_SRC})

target_link_libraries(workrave-qt workrave-frontend-common)
target_link_libraries(workrave-qt workrave-backend)
target_link_libraries(workrave-qt workrave-libs-config)
target_link_libraries(workrave-qt workrave-libs-input-monitor)

if (HAVE_DBUS)
  target_link_libraries(workrave-qt workrave-libs-dbus)
endif()

target_link_libraries(workrave-qt workrave-libs-utils)

link_directories(${GLIB_LIBPATH})
target_link_libraries(workrave-qt ${GLIB_LIBRARIES})

#link_directories(${GDK_LIBPATH})
#target_link_libraries(workrave-qt ${GDK_LIBRARIES})

target_link_libraries(workrave-qt ${Qt5Widgets_LIBRARIES})
target_link_libraries(workrave-qt ${Boost_LIBRARIES})

#qt5_use_modules(helloworld Widgets Sql Network)

if (PLATFORM_OS_UNIX)
  target_link_libraries(workrave-qt ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})

#  add_definitions(-DGNOMELOCALEDIR="${DATADIR}/locale")
#
# Desktop file
intltool_merge_desktop(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave.desktop.in
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
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/resource.rc.in
#    ${CMAKE_CURRENT_BINARY_DIR}/src/resource.rc)
#
#endif(WIN32)

install(TARGETS workrave-qt RUNTIME DESTINATION bin)

# TODO: win32 resource.rc
