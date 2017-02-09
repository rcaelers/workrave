if (HAVE_PANELAPPLET)

set(LOCATION   "${LIBDIR}/libworkrave-applet.so")
set(IN_PROCESS true)
set(VERSION    ${PACKAGE_VERSION})

set(APPLETBINDIR "${LIBDIR}/mate-applets")
set(APPLETDIR    "${LIBDIR}/mate-panel/applets")
set(UIDIR        "${DATADIR}/mate-panel/ui")

set(LOCATION ${APPLETBINDIR})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.WorkraveApplet.mate-panel-applet.in.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet.in
  @ONLY)

intltool_merge_desktop(
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet)
add_custom_target(generate_mate_panel_applet ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet)

set(SRC main.c)

install(FILES v5/workrave-menu.xml DESTINATION ${UIDIR})

add_library(mate-workrave-applet SHARED ${SRC})

install(TARGETS mate-workrave-applet RUNTIME DESTINATION ${APPLETBINDIR} LIBRARY DESTINATION ${APPLETBINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.panel-applet DESTINATION ${PANELDIR})

target_link_libraries(mate-workrave-applet workrave-gtk2-private workrave-libs-utils ${MATE_LIRARIES} ${GTK2_LIBRARIES})

target_include_directories(mate-workrave-applet
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/common/include
  ${MATE_INCLUDE_DIRS}
  ${GTK2_INCLUDE_DIRS}
  )

target_compile_definitions(mate-workrave-applet
  PRIVATE
  -DWORKRAVE_MENU_UI_DIR="${UIDIR}"
  -DGNOMELOCALEDIR="${DATADIR}/locale" 
	-DMATELOCALEDIR="${DATADIR}/locale"
  )
endif()
