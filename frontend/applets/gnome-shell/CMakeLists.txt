set(uuid "workrave@workrave.org")
set(url "http://www.workrave.org")

configure_file(src/metadata.json.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.json)

add_custom_target(generate_gnome_shell_metadata ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/metadata.json)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/metadata.json src/extension.js src/stylesheet.css
  DESTINATION ${DATADIR}/gnome-shell/extensions/${uuid}
  OPTIONAL
  )
