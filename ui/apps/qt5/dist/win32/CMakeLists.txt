include(InstallRequiredSystemLibraries)

set(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")

#set(plugin_dest_dir "lib/plugins")
set(qtconf_dest_dir "bin")

set(IMAGE_PLUGINS QJpegPlugin QGifPlugin)
set(PLATFORM_PLUGINS QMinimalIntegrationPlugin QWindowsIntegrationPlugin)

foreach(plugin ${PLATFORM_PLUGINS})
  get_target_property(_loc Qt5::${plugin} LOCATION)
  message("Plugin ${plugin} is at location ${_loc}")
  if (_loc)
    install(FILES ${_loc} DESTINATION lib/platforms)
  endif()
endforeach()

foreach(plugin ${IMAGE_PLUGINS})
  get_target_property(_loc Qt5::${plugin} LOCATION)
  message("Plugin ${plugin} is at location ${_loc}")
  if (_loc)
    install(FILES ${_loc} DESTINATION lib/imageformats)
  endif()
endforeach()

# Install a qt.conf file.
# This inserts some cmake code into the install script to write the file
install(CODE "
  file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = ../lib/plugins/\")
  " COMPONENT Runtime)

message("boost ${Boost_LIBRARY_DIRS}")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateInstaller.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake"
  @ONLY
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake")
