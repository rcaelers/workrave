if (APPLE)


  set(APPS "\${CMAKE_INSTALL_PREFIX}/Workrave.app")

  include(InstallRequiredSystemLibraries)

  install(CODE "
       cmake_policy(SET CMP0009 NEW)
       cmake_policy(SET CMP0011 NEW)
       file(GLOB_RECURSE PLUGINS \"\${CMAKE_INSTALL_PREFIX}/Workrave.app/Contents/MacOS/*\")
       include(BundleUtilities)
       set(BU_CHMOD_BUNDLE_ITEMS ON)
       set(BU_COPY_FULL_FRAMEWORK_CONTENTS ON)
       fixup_bundle(\"${APPS}\"   \"\${PLUGINS}\"   \"${DIRS}\")
       " COMPONENT Runtime)

  install(FILES "${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns" DESTINATION ${RESOURCESDIR} RENAME "Workrave.icns")

endif()
