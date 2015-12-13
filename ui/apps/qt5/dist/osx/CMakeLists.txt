if (APPLE)
 
  set(APPS "\${CMAKE_INSTALL_PREFIX}/Workrave.app")
  set(PLUGINS "")
  set(DIRS "${Boost_LIBRARY_DIRS};${QT_INSTALL_PREFIX}")

  include(InstallRequiredSystemLibraries)
  
  set(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")
  
  set(plugin_dest_dir "Workrave.app/Contents/MacOS")
  set(qtconf_dest_dir "Workrave.app/Contents/Resources")
  
  install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
  install(DIRECTORY "${QT_PLUGINS_DIR}/platforms" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
  
  # Install a qt.conf file.
  # This inserts some cmake code into the install script to write the file
  install(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = MacOS/plugins/\")
    " COMPONENT Runtime)
  
  install(CODE "
       cmake_policy(SET CMP0009 NEW)
       cmake_policy(SET CMP0011 NEW)
       file(GLOB_RECURSE PLUGINS \"\${CMAKE_INSTALL_PREFIX}/Workrave.app/Contents/MacOS/*\")
       include(BundleUtilities)
       set(BU_CHMOD_BUNDLE_ITEMS ON)
       set(BU_COPY_FULL_FRAMEWORK_CONTENTS ON)
       fixup_bundle(\"${APPS}\"   \"\${PLUGINS}\"   \"${DIRS}\")
       " COMPONENT Runtime)
  
  install(FILES "${CMAKE_SOURCE_DIR}/ui/data/images/osx/workrave.icns" DESTINATION ${DATADIR} RENAME "Workrave.icns")

endif()
