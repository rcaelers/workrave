set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/Workrave.exe")
set(PLUGINS "")
set(DIRS "${Boost_LIBRARY_DIR};${Qt_BIN_DIR};${HOOK_BINARY_DIR}/Release")

include(InstallRequiredSystemLibraries)
  
set(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")
  
set(plugin_dest_dir "lib/plugins")
set(qtconf_dest_dir "bin")
  
install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*d.dll" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/platforms" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*d.dll" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/accessible" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*d.dll" EXCLUDE)
  
# Install a qt.conf file.
# This inserts some cmake code into the install script to write the file
install(CODE "
  file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = ../lib/plugins/\")
  " COMPONENT Runtime)
  
install(CODE "
    file(GLOB_RECURSE PLUGINS \"\${CMAKE_INSTALL_PREFIX}/lib/plugins/*\")
    message(\"plugins \${PLUGINS}\")
    include(BundleUtilities)
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    fixup_bundle(\"${APPS}\"   \"\${PLUGINS}\"   \"${DIRS}\")
    " COMPONENT Runtime)
  
#install(FILES "${CMAKE_SOURCE_DIR}/frontend/data/images/osx/workrave.icns" DESTINATION ${DATADIR} RENAME "Workrave.icns")
