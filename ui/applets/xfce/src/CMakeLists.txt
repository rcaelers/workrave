if (HAVE_XFCE4)

  set(plugindir ${LIBDIR}/xfce4/panel-plugins)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/workrave-xfce-applet.desktop.in.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop.in
    @ONLY)

  intltool_merge_desktop(
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop)
  add_custom_target(generate_xfce_panel_applet ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop)

  set(SRC main.c)

  add_executable(xfce4_workrave_plugin ${SRC})

  install(TARGETS xfce4_workrave_plugin RUNTIME DESTINATION ${plugindir} LIBRARY DESTINATION ${plugindir})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop DESTINATION ${plugindir})

  target_link_libraries(xfce4_workrave_plugin workrave-private workrave-libs-utils ${XFCE4_LIBRARIES})

  target_include_directories(xfce4_workrave_plugin
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/common/include
    ${GTK2_INCLUDE_DIRS}
    ${XFCE4_INCLUDE_DIRS}
    )
endif()
