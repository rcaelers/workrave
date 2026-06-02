find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Quick QuickControls2 QuickWidgets REQUIRED)

# Ask Qt for its authoritative QML import path instead of guessing from Qt6Qml_DIR.
# Some layouts, such as MSYS2, place runtime modules under share/qt6/qml rather than prefix/qml.
execute_process(
  COMMAND ${QT_QTPATHS_EXECUTABLE} --query QT_INSTALL_QML
  OUTPUT_VARIABLE QT_QML_IMPORT_PATH_VALUE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_library(workrave-toolkit-qt OBJECT)

target_sources(workrave-toolkit-qt PRIVATE
  qml/sanctuary.qrc
  QmlAboutDialog.cc
  QmlDailyLimitWindow.cc
  QmlMicroBreakWindow.cc
  QmlPrefsDialog.cc
  QmlPrefsBridges.cc
  QmlPluginPrefBridges.cc
  QmlPreludeWindow.cc
  QmlRestBreakWindow.cc
  QmlTimerBoxView.cc
  DebugDialog.cc
  QmlExercisesDialog.cc
  MainWindow.cc
  StatisticsDialog.cc
  Toolkit.cc
  ToolkitFactory.cc
  ToolkitMenu.cc
  utils/SizeGroup.cc
  utils/Ui.cc
  utils/UiUtil.cc
  utils/qformat.cc
  widgets/Icon.cc
  widgets/StatusIcon.cc
  widgets/TimeBar.cc
  widgets/TimerBoxView.cc
)


if (PLATFORM_OS_UNIX)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitUnix.cc platforms/unix/UnixLocker.cc platforms/unix/WaylandWindowManager.cc)
    if (HAVE_WAYLAND)
     	qt_generate_wayland_protocol_client_sources(workrave-toolkit-qt
         FILES  ${CMAKE_SOURCE_DIR}/ui/app/platforms/unix/protocols/wlr-layer-shell-unstable-v1.xml
                ${CMAKE_SOURCE_DIR}/ui/app/platforms/unix/protocols/xdg-shell.xml)

      target_sources(workrave-toolkit-qt PRIVATE
        platforms/unix/WaylandWindowManager.cc)
    endif()

    # if (HAVE_DBUSMENU)
    #   target_sources(workrave-toolkit-qt PRIVATE platforms/unix/DbusMenu.cc)
    # endif()

    # if (HAVE_APPINDICATOR)
    #   target_sources(workrave-toolkit-qt PRIVATE platforms/unix/AppIndicatorMenu.cc)
    # endif()

    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix/protocols)
    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_BINARY_DIR}/ui/app/platforms/unix)
    target_link_libraries(workrave-toolkit-qt PUBLIC Qt::WaylandClient Wayland::Client Qt::WaylandClientPrivate)
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitWindows.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows)
  target_link_libraries(workrave-toolkit-qt PUBLIC Unfold::unfold cmark webview2 ws2_32 mswsock)
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-toolkit-qt PRIVATE
    ToolkitMacOS.cc
    platforms/macos/MouseMonitor.cc
    platforms/macos/MacOSDesktopWindow.cc
    platforms/macos/MacDockTile.mm)

    set_source_files_properties(ToolkitMacOS.cc platforms/macos/MouseMonitor.cc platforms/macos/MacOSDesktopWindow.cc platforms/macos/MacDockTile.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

if (PLATFORM_OS_UNIX)
  target_sources(workrave-toolkit-qt PRIVATE ToolkitUnix.cc)
  target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/unix)
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-toolkit-qt PRIVATE
    ToolkitMacOS.cc
    platforms/macos/MouseMonitor.cc
    platforms/macos/MacOSDesktopWindow.cc)

    set_source_files_properties(ToolkitMacOS.cc platforms/macos/MouseMonitor.cc platforms/macos/MacOSDesktopWindow.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
    target_include_directories(workrave-toolkit-qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos)
endif()

set(MOC_SOURCES
  QmlAboutDialog.hh
  QmlDailyLimitWindow.hh
  QmlMicroBreakWindow.hh
  QmlPrefsDialog.hh
  QmlPrefsBridges.hh
  QmlPluginPrefBridges.hh
  QmlPreludeWindow.hh
  QmlRestBreakWindow.hh
  QmlTimerBoxView.hh
  DebugDialog.hh
  QmlExercisesDialog.hh
  MainWindow.hh
  StatisticsDialog.hh
  Toolkit.hh
  ToolkitMenu.hh
  utils/SizeGroup.hh
  widgets/StatusIcon.hh
  widgets/TimeBar.hh
  widgets/TimerBoxView.hh
)

target_include_directories(workrave-toolkit-qt
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/utils
  ${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/widgets
  ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
  )

target_compile_definitions(workrave-toolkit-qt PRIVATE "QT_QML_IMPORT_PATH=\"${QT_QML_IMPORT_PATH_VALUE}\"")

target_link_directories(workrave-toolkit-qt PRIVATE
  ${Boost_LIBRARIES_DIRS}
  ${X11_X11_INCLUDE_PATH}
  ${X11_INCLUDE_PATH}
  ${X11_INCLUDE_PATH})

if (HAVE_UNFOLD_AUTO_UPDATE)
  if (PLATFORM_OS_WINDOWS)
    # TODO: target_sources(workrave-toolkit-qt PRIVATE platforms/windows/Edge.cc)
    target_link_libraries(workrave-toolkit-qt PUBLIC Unfold::unfold webview2 ws2_32 mswsock)
  endif()

  target_sources(
    workrave-toolkit-qt
    PRIVATE
    AutoUpdater.cc
    QmlAutoUpdateDialog.cc)

    set(MOC_SOURCES
      ${MOC_SOURCES}
      QmlAutoUpdateDialog.hh
    )
endif()

target_sources(workrave-toolkit-qt PRIVATE ${MOC_SOURCES})


target_link_libraries(workrave-toolkit-qt PRIVATE
  workrave-app
  workrave-libs-audio
  Qt${QT_VERSION_MAJOR}::Quick
  Qt${QT_VERSION_MAJOR}::QuickControls2
  Qt${QT_VERSION_MAJOR}::QuickWidgets
  workrave-libs-config
  workrave-libs-core-next
  workrave-libs-dbus
  workrave-libs-input-monitor
  workrave-libs-session
  workrave-libs-utils
  workrave-libs-ui-prefwidgets
  workrave-libs-ui-prefwidgets-qt
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Svg
  Qt${QT_VERSION_MAJOR}::Xml
  ${LIBINTL_LIBRARIES}
  ${EXTRA_LIBRARIES}
  ${X11_X11_LIB}
  ${X11_Xtst_LIB}
  ${X11_Xss_LIB}
  fmt::fmt)

if (HAVE_UNFOLD_AUTO_UPDATE)
  target_link_libraries(workrave-toolkit-qt PRIVATE workrave-libs-updater)
endif()

if (HAVE_GLIB)
  target_include_directories(workrave-toolkit-qt PRIVATE ${GLIB_INCLUDE_DIRS})
  target_link_directories(workrave-toolkit-qt PRIVATE ${GLIB_LIBPATH})
  target_link_libraries(workrave-toolkit-qt PRIVATE ${GLIB_LIBRARIES})
endif()

if (HAVE_CRASH_REPORT)
  add_executable(WorkraveCrashHandler CrashDialog.cc QmlCrashDialog.cc)

  set(WR_VERSION ${WORKRAVE_VERSION})
  set(WR_RESOURCE_VERSION ${WORKRAVE_RESOURCE_VERSION})
  set(WR_TOP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)

  target_sources(WorkraveCrashHandler PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
    qml/crash.qrc)

  target_include_directories(WorkraveCrashHandler PRIVATE ${GTK_INCLUDE_DIRS})
  target_link_directories(WorkraveCrashHandler PRIVATE ${GTK_LIBRARY_DIRS})
  target_include_directories(WorkraveCrashHandler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/commonui/include)

  target_compile_definitions(WorkraveCrashHandler PRIVATE "QT_QML_IMPORT_PATH=\"${QT_QML_IMPORT_PATH_VALUE}\"")

  target_link_libraries(WorkraveCrashHandler
    PRIVATE
    crashpad::handler_lib
    crashpad::tools
    workrave-libs-utils
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Xml
    ${LIBINTL_LIBRARIES}
    )

  if (MSVC)
    target_link_options(WorkraveCrashHandler PRIVATE "/SUBSYSTEM:WINDOWS")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(WorkraveCrashHandler PROPERTIES COMPILE_FLAGS "-municode")
    set_target_properties(WorkraveCrashHandler PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows -municode")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(WorkraveCrashHandler PROPERTIES COMPILE_FLAGS "-mwindows -municode")
    set_target_properties(WorkraveCrashHandler PROPERTIES LINK_FLAGS "-mwindows -municode")
  endif()

  install(TARGETS WorkraveCrashHandler RUNTIME DESTINATION "${BINDIR}")
endif()
