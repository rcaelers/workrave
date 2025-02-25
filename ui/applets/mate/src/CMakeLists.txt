if (HAVE_MATE)

set(APPLETBINDIR "${LIBDIR}/mate-applets")
set(UIDIR        "${DATADIR}/mate-panel/ui")

set(LOCATION "${CMAKE_INSTALL_PREFIX}/${APPLETBINDIR}/workrave-applet")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/org.workrave.WorkraveApplet.mate-panel-applet.in.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet.in
  @ONLY)

intltool_merge_desktop(
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet.in
  ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet)
add_custom_target(generate_mate_panel_applet ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet)

set(SRC main.c)

install(FILES workrave-menu.xml DESTINATION ${UIDIR})

add_executable(mate-workrave-applet ${SRC})
set_target_properties(mate-workrave-applet PROPERTIES OUTPUT_NAME workrave-applet)

install(TARGETS mate-workrave-applet RUNTIME DESTINATION ${APPLETBINDIR} LIBRARY DESTINATION ${APPLETBINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.workrave.WorkraveApplet.mate-panel-applet DESTINATION ${DATADIR}/mate-panel/applets)

dbus_add_activation_service(${CMAKE_CURRENT_SOURCE_DIR}/org.mate.panel.applet.WorkraveAppletFactory.service.in ${APPLETBINDIR})

target_link_libraries(mate-workrave-applet workrave-private-1.0)

target_link_libraries(mate-workrave-applet
  workrave-libs-utils
  ${MATE_LIBRARIES}
  )

target_include_directories(mate-workrave-applet
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include
  ${MATE_INCLUDE_DIRS}
  )

target_compile_definitions(mate-workrave-applet
  PRIVATE
  -DWORKRAVE_MENU_UI_DIR="${CMAKE_INSTALL_PREFIX}/${UIDIR}"
  -DGNOMELOCALEDIR="${CMAKE_INSTALL_PREFIX}/${DATADIR}/locale"
  -DMATELOCALEDIR="${CMAKE_INSTALL_PREFIX}/${DATADIR}/locale"
  )
endif()
