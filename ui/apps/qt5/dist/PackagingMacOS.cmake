configure_file(${CMAKE_SOURCE_DIR}/ui/apps/qt5/dist/macos/Info.plist.in ${CMAKE_BINARY_DIR}/Info.plist)

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
