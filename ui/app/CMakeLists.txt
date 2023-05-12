add_library(workrave-app OBJECT)

target_sources(workrave-app PRIVATE
  AppHold.cc
  Application.cc
  ApplicationFactory.cc
  GUIConfig.cc
  Menus.cc
  PreferencesRegistry.cc
  SoundTheme.cc
  TimerBoxControl.cc
  )

if (PLATFORM_OS_UNIX)
  target_sources(workrave-app PRIVATE
    ApplicationUnix.cc
  )

  ## TODO
  add_compile_definitions(GNOMELOCALEDIR="${CMAKE_INSTALL_PREFIX}/${DATADIR}/locale")

  # Desktop file
  intltool_merge_desktop(
    ${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.Workrave.desktop)
  add_custom_target(generate_desktop_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.Workrave.desktop)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.Workrave.desktop DESTINATION ${DATADIR}/applications)
  target_include_directories(workrave-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-app PRIVATE
    ApplicationWindows.cc
    platforms/windows/DesktopWindow.cc
    platforms/windows/WindowsAppletWindow.cc
    platforms/windows/WindowsCompat.cc
    platforms/windows/WindowsForceFocus.cc
    platforms/windows/WindowsLocker.cc
    platforms/windows/WindowsStatusIcon.cc
    platforms/windows/WindowsFocusAssist.cc
    platforms/windows/Remote.cc
  )

  if (HAVE_HARPOON)
    target_sources(workrave-app PRIVATE platforms/windows/WindowsHarpoonLocker.cc)
  endif()

  #  target_include_directories(workrave PRIVATE ${CMAKE_SOURCE_DIR}/ui/app/toolkits/gtkmm/src/windows)

  #if (HAVE_HARPOON)
  #  target_link_libraries(workrave PRIVATE harpoon64)
  #  target_include_directories(workrave PRIVATE ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
  #endif()

  target_include_directories(workrave-app
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ui/applets/windows/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows
    )

  if (MSVC)
    target_link_libraries(workrave-app PRIVATE ${WIN32_LIBRARIES})
  endif()
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-app PRIVATE
    ApplicationMacOS.cc
    platforms/macos/MacOSLocker.cc
    platforms/macos/MacOSDock.mm
    #platforms/macos/MacOSAppletWindow.cc
    #platforms/macos/MacOSStatusBarView.mm
    #platforms/macos/MacOSTimeBar.mm
   )

  set_source_files_properties(Locale.cc
                              platforms/macos/MacOSLocker.cc
                              platforms/macos/MacOSDock.mm
                              PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")

  target_include_directories(workrave-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR} DBusGUI)
  target_sources(workrave-app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc GenericDBusApplet.cc)
  dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.Workrave.service.in ${BINDIR})
endif()

if (HAVE_GSETTINGS)
  gsettings_add_schemas(${CMAKE_CURRENT_SOURCE_DIR})
endif()

target_include_directories(workrave-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../libs/commonui/include)

target_link_libraries(workrave-app
  PRIVATE
  Boost::serialization
  workrave-libs-audio
  workrave-libs-config
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  workrave-libs-utils
  workrave-libs-ui-prefwidgets
  workrave-libs-commonui)

if (HAVE_CORE_NEXT)
  target_link_libraries(workrave-app PRIVATE workrave-libs-core-next)
else()
  target_link_libraries(workrave-app PRIVATE workrave-libs-core)
endif()

if (HAVE_CRASH_REPORT)
  target_link_libraries(workrave-app PRIVATE workrave-libs-crash)
endif()

if (HAVE_GLIB)
  target_include_directories(workrave-app PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

install(FILES workrave.metainfo.xml DESTINATION ${DATADIR}/metainfo RENAME org.workrave.Workrave.metainfo.xml)

################################################################################
################################################################################
################################################################################

if (HAVE_APP_QT)
  qt_add_executable(workrave main.cc $<TARGET_OBJECTS:workrave-app>)
else()
  add_executable(workrave main.cc $<TARGET_OBJECTS:workrave-app>)
endif()

set_target_properties(workrave PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
)

target_link_libraries(workrave
  PRIVATE
  workrave-app
  workrave-libs-audio
  workrave-libs-utils
  workrave-libs-ui-prefwidgets)

if (HAVE_CRASH_REPORT)
  target_link_libraries(workrave PRIVATE workrave-libs-crash)
endif()

if (HAVE_CORE_NEXT)
  target_link_libraries(workrave PRIVATE workrave-libs-core-next)
else()
  target_link_libraries(workrave PRIVATE workrave-libs-core)
endif()

if (HAVE_APP_GTK)
  target_include_directories(workrave PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/toolkits/gtkmm)
  target_link_libraries(workrave PRIVATE workrave-toolkit-gtkmm)
  target_sources(workrave PRIVATE $<TARGET_OBJECTS:workrave-toolkit-gtkmm>)
elseif (HAVE_APP_QT)
  target_include_directories(workrave PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/toolkits/qt)
  target_link_libraries(workrave PRIVATE workrave-toolkit-qt)
  target_sources(workrave PRIVATE $<TARGET_OBJECTS:workrave-toolkit-qt>)
endif()

If (PLATFORM_OS_WINDOWS)
  if (CMAKE_CROSSCOMPILING)
    target_link_directories(workrave PRIVATE "${SYS_ROOT}")
  endif()

  set(WR_VERSION ${WORKRAVE_VERSION})
  set(WR_RESOURCE_VERSION ${WORKRAVE_RESOURCE_VERSION})
  set(WR_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  set(WR_CURRENT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

  string(REPLACE "," "." WR_VERSION_VERSION_NUMERIC ${WR_RESOURCE_VERSION})

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/workrave.manifest.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave.manifest)

  target_sources(workrave PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
endif()

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

  if (DUMP_SYMS)
    add_custom_command(TARGET workrave POST_BUILD
                       COMMAND ${DUMP_SYMS} $<TARGET_FILE:workrave> -o ${CMAKE_BINARY_DIR}/workrave.sym)
    add_dependencies(workrave dump_syms)
  endif()

  if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave.debug DESTINATION ${BINDIR})
  endif()
endif()

if (APPLE)
  set_target_properties(workrave
    PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}"
    MACOSX_BUNDLE_ICON_FILE Workrave.icns
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.workrave.${PROJECT_NAME}"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${WORKRAVE_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${WORKRAVE_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${WORKRAVE_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2001-2023 Rob Caelers, Raymond Penners, Ray Satiro"
    )

  set_source_files_properties(BreakWindow.cc PreludeWindow.cc MouseMonitor.cc ToolkitPlatformMac.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

configure_file(project.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/Workrave.vcxproj.user @ONLY)

if (PLATFORM_OS_UNIX)
  ## TODO
  add_compile_definitions(GNOMELOCALEDIR="${CMAKE_INSTALL_PREFIX}/${DATADIR}/locale")
endif()

if (PLATFORM_OS_WINDOWS)
  if (MSVC)
    target_link_libraries(workrave PRIVATE ${WIN32_LIBRARIES})
  endif()

  if (MINGW)
    target_link_libraries(workrave PRIVATE winmm shlwapi kernel32 user32 gdi32 winspool comdlg32  advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)
    target_link_libraries(workrave PRIVATE dxerr8 dsound dxguid comctl32)
  endif()
  if (MSVC)
    target_link_libraries(workrave PRIVATE wtsapi32 dsound dxguid)
  endif()

  if (PLATFORM_OS_WINDOWS_NATIVE)
    set_target_properties(workrave PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
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
      ##TODO: set_target_properties(workrave PROPERTIES COMPILE_FLAGS "-Wl,--subsystem,windows")
      set_target_properties(workrave PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set_target_properties(workrave PROPERTIES COMPILE_FLAGS "-mwindows")
      set_target_properties(workrave PROPERTIES LINK_FLAGS "-mwindows")
    endif()
  endif()
endif()

install(TARGETS workrave RUNTIME DESTINATION ${BINDIR} BUNDLE DESTINATION .)

add_subdirectory(toolkits)
