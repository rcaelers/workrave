if (HAVE_PANELAPPLET)

set(LOCATION   "${LIBDIR}/libworkrave-applet.so")
set(IN_PROCESS true)
set(VERSION    ${PACKAGE_VERSION})
set(APPLETDIR  "${DATADIR}/gnome-panel/4.0/applets")
set(UIDIR      "${DATADIR}/gnome-panel/ui")

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

if (HAVE_PANELAPPLET5)
  set(SRC ${SRC} v5/WorkraveApplet.c)
  install(FILES v5/workrave-gnome-applet-menu.xml DESTINATION ${UIDIR})
  set(PANELDIR "${DATADIR}/gnome-panel/4.0/applets")
  pkgconfig_get_var(PANELDIR libpanel-applet libpanel_applet_dir)

elseif (HAVE_PANELAPPLET4)
  set(SRC ${SRC} v4/WorkraveApplet.c)
  install(FILES v4/workrave-gnome-applet-menu.xml DESTINATION ${UIDIR})
  set(PANELDIR "${DATADIR}/gnome-panel/4.0/applets")
endif()

add_library(workrave-applet SHARED ${SRC})

install(TARGETS workrave-applet RUNTIME DESTINATION ${APPLETDIR} LIBRARY DESTINATION ${APPLETDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet DESTINATION ${CMAKE_INSTALL_PREFIX}/${PANELDIR})

target_link_libraries(workrave-applet workrave-private workrave-libs-utils)

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
