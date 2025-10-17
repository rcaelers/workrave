add_library(workrave-libs-ui-prefwidgets-gtkmm STATIC)

if (HAVE_APP_GTK)
  target_sources(workrave-libs-ui-prefwidgets-gtkmm PRIVATE
    BoxWidget.cc
    Builder.cc
    ContainerWidget.cc
    ChoiceWidget.cc
    EntryWidget.cc
    FrameWidget.cc
    Hig.cc
    ToggleWidget.cc
    SpinWidget.cc
    SliderWidget.cc
    SizeGroup.cc
    TimeEntryWidget.cc
    Widget.cc
    )

  target_include_directories(
    workrave-libs-ui-prefwidgets-gtkmm
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ui/libs/prefwidgets/toolkits/gtkmm/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ui/libs/prefwidgets/toolkits/gtkmm/include/ui/prefwidgets/gtkmm
    ${GTK_INCLUDE_DIRS}
    )

  target_link_libraries(workrave-libs-ui-prefwidgets-gtkmm PRIVATE ${GTK_LIBRARIES})
  target_link_libraries(workrave-libs-ui-prefwidgets-gtkmm PRIVATE ${EXTRA_LIBRARIES})
  target_link_directories(workrave-libs-ui-prefwidgets-gtkmm PUBLIC ${GTK_LIBRARY_DIRS})
endif()

If (PLATFORM_OS_WINDOWS)
  if (CMAKE_CROSSCOMPILING)
    target_link_directories(workrave-libs-ui-prefwidgets-gtkmm PRIVATE "${SYS_ROOT}")
  endif()
endif()

target_link_libraries(workrave-libs-ui-prefwidgets-gtkmm
  PUBLIC
  PRIVATE
  workrave-libs-config
  workrave-libs-utils
  workrave-libs-ui-prefwidgets)

# if (PLATFORM_OS_UNIX)
#   target_link_libraries(workrave-libs-ui-prefwidgets-gtkmm PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
# endif()

