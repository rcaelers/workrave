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

install(FILES "${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns" DESTINATION ${RESOURCESDIR} RENAME "Workrave.icns")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(NO_STRIP "-no-strip")
endif()

get_target_property(FMT_PATH fmt::fmt LOCATION)
get_filename_component(FMT_PATH "${FMT_PATH}" DIRECTORY)

get_target_property(SPDLOG_PATH spdlog::spdlog LOCATION)
get_filename_component(SPDLOG_PATH "${SPDLOG_PATH}" DIRECTORY)

INSTALL(CODE "
    message(\"-- Installing dependencies...\")
    message(\"-- $<$<BOOL:${SPDLOG_PATH}>:-libpath=${SPDLOG_PATH}/lib>\")
    execute_process(COMMAND ln -s ./Frameworks ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/lib)
    execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app -verbose=1
           \"$<$<BOOL:${BOOST_ROOT}>:-libpath=${BOOST_ROOT}/lib>\"
           \"$<$<BOOL:${FMT_PATH}>:-libpath=${FMT_PATH}>\"
           \"$<$<BOOL:${SPDLOG_PATH}>:-libpath=${SPDLOG_PATH}>\"
           \"$<$<BOOL:${WORKRAVE_SIGN_IDENTITY}>:-sign-for-notarization=${WORKRAVE_SIGN_IDENTITY}>\" ${NO_STRIP} -always-overwrite)
    ")
