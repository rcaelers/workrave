if (HAVE_PANELAPPLET)

set(APPLETDIR  "${LIBDIR}/workrave")
set(UIDIR      "${DATADIR}/gnome-panel/ui")

set(LOCATION   "${CMAKE_INSTALL_PREFIX}/${APPLETDIR}/libworkrave-applet.so")
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

gdbus_codegen(
  "workrave-gnome-applet-generated.h workrave-gnome-applet-generated.c"
  workrave-gnome-applet-generated
  org.workrave.
  Workrave
  ${CMAKE_CURRENT_SOURCE_DIR}/workrave-gnome-applet.xml
  )

set(SRC workrave-gnome-applet-generated.c)

if (HAVE_LIBGNOME_PANEL)
  set(SRC ${SRC} libgnome-panel/WorkraveApplet.c libgnome-panel/WorkraveModule.c)
  install(FILES libgnome-panel/workrave-gnome-applet-menu.xml DESTINATION ${UIDIR})
  set(PANELDIR "${DATADIR}/gnome-panel/applets")
  pkgconfig_get_var(PANELDIR libgnome-panel modulesdir)
elseif (HAVE_PANELAPPLET5)
  set(SRC ${SRC} v5/WorkraveApplet.c)
  install(FILES v5/workrave-gnome-applet-menu.xml DESTINATION ${UIDIR})
  set(PANELDIR "${DATADIR}/gnome-panel/applets")
  pkgconfig_get_var(PANELDIR libpanel-applet libpanel_applet_dir)
elseif (HAVE_PANELAPPLET4)
  set(SRC ${SRC} v4/WorkraveApplet.c)
  install(FILES v4/workrave-gnome-applet-menu.xml DESTINATION ${UIDIR})
  set(PANELDIR "${DATADIR}/gnome-panel/4.0/applets")
endif()

add_library(workrave-applet SHARED ${SRC})

install(TARGETS workrave-applet RUNTIME DESTINATION ${APPLETDIR} LIBRARY DESTINATION ${APPLETDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet DESTINATION ${CMAKE_INSTALL_PREFIX}/${PANELDIR})

target_link_libraries(workrave-applet workrave-private-1.0 workrave-libs-utils)

target_include_directories(workrave-applet
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/common/include
  ${GTK_INCLUDE_DIRS}
  ${PANELAPPLET_INCLUDE_DIRS}
  )

target_compile_definitions(workrave-applet
  PRIVATE
  -DWORKRAVE_UIDATADIR="${UIDIR}"
  -DGNOMELOCALEDIR="${DATADIR}/locale" 
  )
endif()
