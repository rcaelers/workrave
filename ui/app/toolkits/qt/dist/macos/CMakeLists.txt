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

set(_FMT_LIBPATH "")
set(_SPDLOG_LIBPATH "")
if(TARGET fmt::fmt)
    get_target_property(_fmt_type fmt::fmt TYPE)
    if(NOT _fmt_type STREQUAL "INTERFACE_LIBRARY")
        set(_FMT_LIBPATH "-libpath=$<TARGET_FILE_DIR:fmt::fmt>")
    endif()
endif()
if(TARGET spdlog::spdlog)
    get_target_property(_spdlog_type spdlog::spdlog TYPE)
    if(NOT _spdlog_type STREQUAL "INTERFACE_LIBRARY")
        set(_SPDLOG_LIBPATH "-libpath=$<TARGET_FILE_DIR:spdlog::spdlog>")
    endif()
endif()

INSTALL(CODE "
    message(\"-- Installing dependencies...\")
    set(_app ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app)
    execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf \${_app}/Contents/Frameworks)
    execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf \${_app}/Contents/PlugIns)
    execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf \${_app}/Contents/lib)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ./Frameworks \${_app}/Contents/lib)
    execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} \${_app} -verbose=1
           \"$<$<BOOL:${BOOST_ROOT}>:-libpath=${BOOST_ROOT}/lib>\"
           \"${_FMT_LIBPATH}\"
           \"${_SPDLOG_LIBPATH}\"
           \"$<$<BOOL:${WORKRAVE_SIGN_IDENTITY}>:-sign-for-notarization=${WORKRAVE_SIGN_IDENTITY}>\" ${NO_STRIP})
    if(NOT \"${WORKRAVE_SIGN_IDENTITY}\")
        message(\"-- Ad-hoc signing app bundle...\")
        execute_process(COMMAND codesign --force --deep --sign - \${_app})
    endif()
    ")
