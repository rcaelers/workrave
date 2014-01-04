set(APPS "\${CMAKE_INSTALL_PREFIX}/Workrave.app")
set(PLUGINS "")
set(DIRS ${QT_LIBRARY_DIRS})

include(InstallRequiredSystemLibraries)


SET(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")

SET(plugin_dest_dir "Workrave.app/Contents/MacOS")
SET(qtconf_dest_dir "Workrave.app/Contents/Resources")

message(${plugin_dest_dir})

INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/platforms" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/accessible" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)

#file(GLOB_RECURSE PLUGINS "${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
#message("plugs ${PLUGINS}")

# Install a qt.conf file.
# This inserts some cmake code into the install script to write the file
INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = MacOS/plugins/\")
    " COMPONENT Runtime)

install(CODE "
       file(GLOB_RECURSE PLUGINS \"\${CMAKE_INSTALL_PREFIX}/Workrave.app/Contents/MacOS/*\")
       message(\"plugins \${PLUGINS}\")
       include(BundleUtilities)
       set(BU_CHMOD_BUNDLE_ITEMS ON)
       fixup_bundle(\"${APPS}\"   \"\${PLUGINS}\"   \"${DIRS}\")
       " COMPONENT Runtime)

install(FILES "${CMAKE_SOURCE_DIR}/frontend/data/images/osx/workrave.icns" DESTINATION ${DATADIR} RENAME "Workrave.icns")

