if (UNIX AND NOT APPLE)

  set(uuid "workrave@workrave.org")
  set(url "http://www.workrave.org")
  set(localedir ${CMAKE_INSTALL_PREFIX}/${LOCALEDIR})

  configure_file(metadata.json.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.json)

  add_custom_target(generate_gnome_shell_metadata ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/metadata.json)

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/metadata.json extension.js stylesheet.css
    DESTINATION ${DATADIR}/gnome-shell/extensions/${uuid}
    OPTIONAL
    )

endif()
