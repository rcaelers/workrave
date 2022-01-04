configure_file(${CMAKE_SOURCE_DIR}/ui/app/toolkits/gtkmm/dist/macos/Info.plist.in ${CMAKE_BINARY_DIR}/Info.plist)

list(APPEND CPACK_GENERATOR "DragNDrop")

set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns)

set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}_${VERSION}")
set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/DS_Store")
set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/dmg_background.png")
set(CPACK_DMG_FORMAT "UDBZ")

set(CPACK_BUNDLE_ICON ${CMAKE_PACKAGE_ICON})
set(CPACK_BUNDLE_NAME "${PROJECT_NAME}_${VERSION}")
set(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/Info.plist")
set(CPACK_SYSTEM_NAME "OSX")

include(InstallRequiredSystemLibraries)

install(FILES "${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns" DESTINATION ${RESOURCESDIR} RENAME "Workrave.icns")

if (DEFINED ENV{JHBUILD_PREFIX} AND NOT DEFINED SYSROOT)
  set(SYSROOT $ENV{JHBUILD_PREFIX})

  install(DIRECTORY ${SYSROOT}/etc/gtk-3.0 DESTINATION ${RESOURCESDIR}/etc)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/settings.ini DESTINATION ${RESOURCESDIR}/etc/gtk-3.0)

  install(DIRECTORY ${SYSROOT}/share/themes DESTINATION ${RESOURCESDIR}/share)
  install(DIRECTORY ${SYSROOT}/share/icons/Adwaita DESTINATION ${DATADIR}/icons)
  install(CODE "execute_process(COMMAND gtk-update-icon-cache \${DATADIR}/icons/Adwaita)")

  install(DIRECTORY ${SYSROOT}/lib/gtk-3.0/3.0.0/immodules DESTINATION ${RESOURCESDIR}/lib/gtk-3.0/3.0.0/)
  install(DIRECTORY ${SYSROOT}/lib/gdk-pixbuf-2.0/2.10.0 DESTINATION ${RESOURCESDIR}/lib/gdk-pixbuf-2.0/)

  install(CODE "
    file(READ \"${CMAKE_INSTALL_PREFIX}/${RESOURCESDIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache\" filedata)
    string(REGEX REPLACE \"lib/gdk-pixbuf-2.0/2.10.0/loaders/\" \"Resources/lib/gdk-pixbuf-2.0/2.10.0/loaders/\" filedata \"\${filedata}\")
    file(WRITE \"${CMAKE_INSTALL_PREFIX}/${RESOURCESDIR}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache\" \"\${filedata}\")
    " COMPONENT Runtime)

  install(DIRECTORY ${SYSROOT}/share/glib-2.0/schemas/ DESTINATION ${RESOURCESDIR}/share/glib-2.0/schemas/ FILES_MATCHING PATTERN "org.gtk*.xml")
  install(CODE "execute_process (COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/${RESOURCESDIR}/share/glib-2.0/schemas\")")
endif()

set(APPS "${CMAKE_INSTALL_PREFIX}/Workrave.app")
set(PLUGINS "")
set(DIRS "${Boost_LIBRARY_DIRS};${spdlog_BINARY_DIR}")

install(CODE "
  cmake_policy(SET CMP0009 NEW)
  cmake_policy(SET CMP0011 NEW)
  file(GLOB_RECURSE PLUGINS \"${CMAKE_INSTALL_PREFIX}/Workrave.app/Contents/Resources/lib/*.so\")
  include(BundleUtilities)
  set(BU_CHMOD_BUNDLE_ITEMS ON)
  set(BU_COPY_FULL_FRAMEWORK_CONTENTS OFF)
  fixup_bundle(\"${APPS}\"   \"\${PLUGINS}\"   \"${DIRS}\")
  " COMPONENT Runtime)

