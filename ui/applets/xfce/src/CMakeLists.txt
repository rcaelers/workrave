if (HAVE_XFCE4)

  pkgconfig_get_var(XFCE4LIBDIR libxfce4panel-2.0 libdir)

  intltool_merge_desktop(
    ${CMAKE_CURRENT_SOURCE_DIR}/workrave-xfce-applet-2.0.desktop.in.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop)
  add_custom_target(generate_xfce_panel_applet ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop)

  set(SRC main.c)

  add_library(workrave-xfce SHARED ${SRC})

  install(TARGETS workrave-xfce DESTINATION ${XFCE4LIBDIR}/xfce4/panel/plugins)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop DESTINATION ${DATADIR}/xfce4/panel/plugins)

  set_target_properties(workrave-xfce
    PROPERTIES
    OUTPUT_NAME workrave-plugin)

  target_link_libraries(workrave-xfce
    workrave-private-1.0
    workrave-libs-utils
    ${XFCE4_LIBRARIES})

  target_include_directories(workrave-xfce
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include
    ${XFCE4_INCLUDE_DIRS}
    )
endif()
