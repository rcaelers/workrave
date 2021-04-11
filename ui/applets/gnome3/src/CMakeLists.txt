if (HAVE_PANELAPPLET)

set(APPLET_DIR "${LIBDIR}/workrave")
set(XML_UI_DIR "${DATADIR}/gnome-panel/ui")

if (HAVE_LIBGNOME_PANEL)
  set(SRC libgnome-panel/WorkraveApplet.c libgnome-panel/WorkraveModule.c)
  set(XML_UI_DIR "${DATADIR}/workrave/ui")
  set(APPLET_DIR "${DATADIR}/gnome-panel/modules")
  pkgconfig_get_var(APPLET_DIR libgnome-panel modulesdir)
  install(FILES libgnome-panel/workrave-gnome-applet-menu.xml DESTINATION ${XML_UI_DIR})
elseif (HAVE_PANELAPPLET5)
  set(SRC v5/WorkraveApplet.c)
  set(CONFIG_DIR "${DATADIR}/gnome-panel/applets")
  pkgconfig_get_var(CONFIG_DIR libpanel-applet libpanel_applet_dir)
  install(FILES v5/workrave-gnome-applet-menu.xml DESTINATION ${XML_UI_DIR})
elseif (HAVE_PANELAPPLET4)
  set(SRC v4/WorkraveApplet.c)
  set(CONFIG_DIR "${DATADIR}/gnome-panel/4.0/applets")
  install(FILES v4/workrave-gnome-applet-menu.xml DESTINATION ${XML_UI_DIR})
endif()

set(LOCATION   "${CMAKE_INSTALL_PREFIX}/${APPLET_DIR}/libworkrave-applet.so")
set(IN_PROCESS true)
set(VERSION    ${PACKAGE_VERSION})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.WorkraveApplet.panel-applet.in.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet.in
  @ONLY)

intltool_merge_desktop(
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet)
add_custom_target(generate_gnome3_panel_applet ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet)

add_library(workrave-applet SHARED ${SRC})

target_link_libraries(workrave-applet
  workrave-private-1.0
  workrave-libs-utils)

target_include_directories(workrave-applet
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/common/include
  ${GTK_INCLUDE_DIRS}
  ${PANELAPPLET_INCLUDE_DIRS}
  )

target_compile_definitions(workrave-applet
  PRIVATE
  -DWORKRAVE_UIDATADIR="${XML_UI_DIR}"
  -DGNOMELOCALEDIR="${DATADIR}/locale"
  )

install(TARGETS workrave-applet RUNTIME DESTINATION ${APPLET_DIR} LIBRARY DESTINATION ${APPLET_DIR})

if (CONFIG_DIR)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet DESTINATION ${CONFIG_DIR})
endif()

endif()
