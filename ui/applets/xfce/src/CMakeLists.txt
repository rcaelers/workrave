if (HAVE_XFCE4)

  # TOD: check for Gtk2/Gtk3, check if applet must be shared library or executable.
  set(XFCE_EXEC ${CMAKE_INSTALL_PREFIX}/${LIBDIR}/xfce4/panel-plugins/xfce4-workrave-plugin)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/workrave-xfce-applet-1.0.desktop.in.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop.in
    @ONLY)

  intltool_merge_desktop(
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop)
  add_custom_target(generate_xfce_panel_applet ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop)

  set(SRC main.c)

  add_executable(xfce4-workrave-plugin ${SRC})

  install(TARGETS xfce4-workrave-plugin DESTINATION ${LIBDIR}/xfce4/panel-plugins)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/workrave-xfce-applet.desktop DESTINATION ${DATADIR}/xfce4/panel-plugins)

  target_link_libraries(xfce4-workrave-plugin workrave-gtk2-private-1.0 workrave-libs-utils ${XFCE4_LIBRARIES} )

  target_include_directories(xfce4-workrave-plugin
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/common/include
    ${GTK2_INCLUDE_DIRS}
    ${XFCE4_INCLUDE_DIRS}
    )
endif()
