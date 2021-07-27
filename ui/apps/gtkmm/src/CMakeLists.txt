set(SRC
  AppletControl.cc
  AppletWindow.cc
  BreakWindow.cc
  DailyLimitWindow.cc
  DataConnector.cc
  DebugDialog.cc
  EventButton.cc
  EventImage.cc
  EventLabel.cc
  ExercisesDialog.cc
  ExercisesPanel.cc
  Frame.cc
  #Grab.cc
  GUI.cc
  GtkUtil.cc
  HeadInfo.cc
  Hig.cc
  IconListCellRenderer.cc
  IconListNotebook.cc
  Locale.cc
  MainGtkMenu.cc
  MainWindow.cc
  Menus.cc
  MicroBreakWindow.cc
  PreferencesDialog.cc
  PreludeWindow.cc
  RestBreakWindow.cc
  StatisticsDialog.cc
  StatusIcon.cc
  Text.cc
  TimeBar.cc
  TimeEntry.cc
  TimerBoxGtkView.cc
  TimerBoxPreferencePage.cc
  TimerPreferencesPanel.cc
  Ui.cc
  WindowHints.cc
  main.cc
  )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    unix/X11SystrayAppletWindow.cc
    unix/desktop-window.c
    unix/gtktrayicon3.c
    #unix/UnixGrab.cc
    )

    if (HAVE_INDICATOR)
      set(SRC ${SRC}
        unix/IndicatorAppletMenu.cc
      )
    endif()

    include_directories(${CMAKE_SOURCE_DIR}/ui/apps/gtkmm/src/unix)
endif()

if (PLATFORM_OS_WINDOWS)
  set(SRC ${SRC}
    win32/DesktopWindow.cc
    win32/W32AppletMenu.cc
    win32/W32AppletWindow.cc
    win32/W32Compat.cc
    win32/W32ForceFocus.cc
    win32/W32StatusIcon.cc
    win32/W32TrayMenu.cc
    #win32/W32Grab.cc
    win32/eggsmclient-win32.c
    win32/eggsmclient.c
    )

  if (HAVE_HARPOON)
    #set(SRC ${SRC} win32/W32GrabHarpoon.cc)
  endif()

  set(WORKRAVE_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  set(WORKRAVE_VERSION ${VERSION})
  set(WORKRAVE_RESOURCE_VERSION ${RESOURCE_VERSION})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)

  if (MINGW)
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource_rc.o
                         COMMAND ${CMAKE_RC_COMPILER} -o ${CMAKE_CURRENT_BINARY_DIR}/resource_rc.o
                                                      -i ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
                         DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
                        )
      set(SRC ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/resource_rc.o)
  endif()

  include_directories(${CMAKE_SOURCE_DIR}/ui/apps/gtkmm/src/win32)
endif()

if (PLATFORM_OS_MACOSOS)
  set(SRC ${SRC}
    macos/MacOSAppletWindow.cc
    macos/MacOSGtkMenu.cc
    macos/MacOSUtil.cc
    macos/MacOSStatusBarView.mm
    macos/MacOSTimeBar.mm
    )

  set_source_files_properties(macos/MacOSAppletWindow.cc macos/MacOSGtkMenu.cc macos/MacOSUtil.cc Locale.cc AppletControl.cc macos/MacOSTimeBar.mm macos/MacOSStatusBarView.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")

  include_directories(${CMAKE_SOURCE_DIR}/ui/apps/gtkmm/src/macos)
endif()

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR} DBusGUI)

  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
    GenericDBusApplet.cc
    )

  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in ${BINDIR})
endif()

if (HAVE_GSETTINGS)
  gsettings_add_schemas(${CMAKE_CURRENT_SOURCE_DIR})
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/ui/apps/gtkmm/src
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
  ${GTK_INCLUDE_DIRS}
  ${INDICATOR_INCLUDE_DIRS}
  )

link_directories(${GTK_LIBRARY_DIRS})
link_directories(${INDICATOR_LIBPATH})

If (PLATFORM_OS_WINDOWS)
  link_directories("${HOOK_BINARY_DIR}")
  if (CMAKE_CROSSCOMPILING OR MSYS)
    link_directories("${SYS_ROOT}")
  endif()
endif()

add_executable(workrave ${SRC})

target_link_libraries(workrave
  workrave-ui-common
  workrave-libs-audio
  workrave-libs-config
  workrave-libs-core
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  #workrave-libs-updater
  workrave-libs-utils)

if (MINGW)
  add_custom_command(TARGET workrave POST_BUILD
      # COMMAND ${CMAKE_NM} -nosC --line-numbers $<TARGET_FILE:workrave> > ${CMAKE_CURRENT_BINARY_DIR}/workrave.debug
      COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:workrave>" workrave-debug.exe
      COMMAND ${CMAKE_OBJCOPY} --only-keep-debug
          $<TARGET_FILE:workrave> ${CMAKE_CURRENT_BINARY_DIR}/workrave.debug
      COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded
          $<TARGET_FILE:workrave>
      COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=workrave.debug
          $<TARGET_FILE:workrave>
  )

  add_custom_command(TARGET workrave POST_BUILD
    COMMAND dump_syms $<TARGET_FILE:workrave> -o ${CMAKE_CURRENT_BINARY_DIR}/workrave.sym
  )

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.debug DESTINATION ${BINDIR})
  endif()
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.sym DESTINATION .)
endif()

target_link_libraries(workrave ${GTK_LIBRARIES})
target_link_libraries(workrave ${Boost_LIBRARIES})
target_link_libraries(workrave ${EXTRA_LIBRARIES})

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
  if (HAVE_HARPOON)
    target_link_libraries(workrave harpoon64)
    include_directories(${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
  endif()

  include_directories(
    ${CMAKE_SOURCE_DIR}/ui/applets/win32/include
    ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
    )

  if (MSVC)
    target_link_libraries(workrave ${WIN32_LIBRARIES})
  endif()

  if (MINGW)
    target_link_libraries(workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)
    target_link_libraries(workrave dxerr8 dsound dxguid)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      target_compile_options(workrave PRIVATE -gcodeview)
      target_link_options(workrave PRIVATE -Wl,-pdb=)
    endif()
  else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      target_compile_options(workrave PRIVATE -gcodeview)
      target_link_options(workrave PRIVATE -Wl,-pdb=)
      set_target_properties(workrave PROPERTIES COMPILE_FLAGS "-Wl,--subsystem,windows")
      set_target_properties(workrave PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set_target_properties(workrave PROPERTIES COMPILE_FLAGS "-mwindows")
      set_target_properties(workrave PROPERTIES LINK_FLAGS "-mwindows")
    endif()
  endif()

  if (HAVE_CRASH_REPORT)
    target_link_libraries(workrave workrave-libs-crash)
  endif()

endif(WIN32)

install(TARGETS workrave RUNTIME DESTINATION ${BINDIR})

if (HAVE_CRASHPAD)
  add_executable(WorkraveCrashHandler CrashDialog.cc)

  target_include_directories(WorkraveCrashHandler PRIVATE ${GTK_INCLUDE_DIRS})
  target_link_directories(WorkraveCrashHandler PRIVATE ${GTK_LIBRARY_DIRS})

  target_link_libraries(WorkraveCrashHandler
    PRIVATE
    workrave-ui-common
    workrave-libs-utils
    crashpad::handler_lib
    crashpad::tools
    ${GTK_LIBRARIES}
    )

  if (MSVC)
    target_link_options(WorkraveCrashHandler PRIVATE "/SUBSYSTEM:WINDOWS")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(WorkraveCrashHandler PROPERTIES COMPILE_FLAGS "-Wl,--subsystem,windows  -municode")
    set_target_properties(WorkraveCrashHandler PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows  -municode")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(WorkraveCrashHandler PROPERTIES COMPILE_FLAGS "-mwindows  -municode")
    set_target_properties(WorkraveCrashHandler PROPERTIES LINK_FLAGS "-mwindows -municode")
  endif()

  install(TARGETS WorkraveCrashHandler RUNTIME DESTINATION "${BINDIR}")
endif()
