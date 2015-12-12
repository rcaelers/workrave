include(InstallRequiredSystemLibraries)

message("MS VC : ${MSVC14_CRT_DIR}")

set(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")

#set(plugin_dest_dir "lib/plugins")
set(qtconf_dest_dir "bin")

install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION lib/plugins COMPONENT Runtime PATTERN "*d.*" EXCLUDE PATTERN "*.dll")
install(DIRECTORY "${QT_PLUGINS_DIR}/platforms" DESTINATION lib/plugins COMPONENT Runtime PATTERN "*d.*" EXCLUDE PATTERN "*.dll")
#install(DIRECTORY "${QT_PLUGINS_DIR}/accessible" DESTINATION lib/plugins COMPONENT Runtime PATTERN "*d.*" EXCLUDE PATTERN "*.dll")

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
  ESCAPE_QUOTES
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake")
