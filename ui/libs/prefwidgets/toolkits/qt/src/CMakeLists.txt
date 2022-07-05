add_library(workrave-libs-ui-prefwidgets-qt STATIC)

if (HAVE_APP_QT)

  set(MOC_SOURCES
    ../include/ui/prefwidgets/qt/BoxWidget.hh
    ../include/ui/prefwidgets/qt/ContainerWidget.hh
    ../include/ui/prefwidgets/qt/ChoiceWidget.hh
    ../include/ui/prefwidgets/qt/FrameWidget.hh
    ../include/ui/prefwidgets/qt/ToggleWidget.hh
    ../include/ui/prefwidgets/qt/SpinWidget.hh
    ../include/ui/prefwidgets/qt/SliderWidget.hh
    ../include/ui/prefwidgets/qt/SizeGroup.hh
    ../include/ui/prefwidgets/qt/TimeEntryWidget.hh
    ../include/ui/prefwidgets/qt/Widget.hh
  )

  qt_wrap_cpp(MOC_SRC ${MOC_SOURCES})

  target_sources(workrave-libs-ui-prefwidgets-qt PRIVATE
    BoxWidget.cc
    Builder.cc
    ContainerWidget.cc
    ChoiceWidget.cc
    FrameWidget.cc
    ToggleWidget.cc
    SpinWidget.cc
    SliderWidget.cc
    SizeGroup.cc
    TimeEntryWidget.cc
    Widget.cc
    ${MOC_SRC}
    )

  target_include_directories(
    workrave-libs-ui-prefwidgets-qt
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ui/libs/prefwidgets/toolkits/qt/include
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/libs/prefwidgets/toolkits/qt/include/ui/prefwidgets/qt
    )

  target_link_libraries(workrave-libs-ui-prefwidgets-qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
  target_link_libraries(workrave-libs-ui-prefwidgets-qt PRIVATE ${EXTRA_LIBRARIES})
endif()

If (PLATFORM_OS_WINDOWS)
  if (CMAKE_CROSSCOMPILING)
    target_link_directories(workrave-libs-ui-prefwidgets-qt PRIVATE "${SYS_ROOT}")
  endif()
endif()

target_link_libraries(workrave-libs-ui-prefwidgets-qt
  PUBLIC
  PRIVATE
  workrave-libs-config
  workrave-libs-utils
  workrave-libs-ui-prefwidgets)

# if (PLATFORM_OS_UNIX)
#   target_link_libraries(workrave-libs-ui-prefwidgets-qt PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
# endif()

