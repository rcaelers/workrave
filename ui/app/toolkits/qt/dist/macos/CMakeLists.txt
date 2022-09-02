configure_file(${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/dist/macos/Info.plist.in ${CMAKE_BINARY_DIR}/Info.plist)

list(APPEND CPACK_GENERATOR "DragNDrop")

set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns)

set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}_${VERSION}")
set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/macos/DS_Store")
set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/macos/dmg_background.png")
set(CPACK_DMG_FORMAT "UDBZ")

set(CPACK_BUNDLE_ICON ${CMAKE_PACKAGE_ICON})
set(CPACK_BUNDLE_NAME "${PROJECT_NAME}_${VERSION}")
set(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/Info.plist")
set(CPACK_SYSTEM_NAME "OSX")

include(InstallRequiredSystemLibraries)

set(APPS "${CMAKE_INSTALL_PREFIX}/Workrave.app")
set(PLUGINS "")
set(DIRS "${Boost_LIBRARY_DIRS};${QT_INSTALL_PREFIX};${spdlog_BINARY_DIR}")

set(plugin_dest_dir "Workrave.app/Contents/Plugins")
set(qtconf_dest_dir "Workrave.app/Contents/Resources")

install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"     DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats"  DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/iconengines"   DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/styles"        DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)

install(CODE "
  file(WRITE \"${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = ../Plugins/\")
  " COMPONENT Runtime)

install(CODE "
  cmake_policy(SET CMP0009 NEW)
  cmake_policy(SET CMP0011 NEW)
  file(GLOB_RECURSE PLUGINS \"${CMAKE_INSTALL_PREFIX}/Workrave.app/Contents/PlugIns/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
  include(BundleUtilities)
  set(BU_CHMOD_BUNDLE_ITEMS ON)
  set(BU_COPY_FULL_FRAMEWORK_CONTENTS OFF)
  fixup_bundle(\"${APPS}\"   \"\${PLUGINS}\"   \"${DIRS}\")
  " COMPONENT Runtime)

  install(FILES "${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns" DESTINATION ${RESOURCESDIR} RENAME "Workrave.icns")
