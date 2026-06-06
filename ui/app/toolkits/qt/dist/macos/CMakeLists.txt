configure_file(${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/dist/macos/Info.plist.in ${CMAKE_BINARY_DIR}/Info.plist)

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

install(FILES "${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns" DESTINATION ${RESOURCESDIR} RENAME "Workrave.icns")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(NO_STRIP "-no-strip")
    set(_debug_entitlements "${CMAKE_CURRENT_SOURCE_DIR}/entitlements_debug.plist")
endif()

get_target_property(FMT_PATH fmt::fmt LOCATION)
get_filename_component(FMT_PATH "${FMT_PATH}" DIRECTORY)

get_target_property(SPDLOG_PATH spdlog::spdlog LOCATION)
get_filename_component(SPDLOG_PATH "${SPDLOG_PATH}" DIRECTORY)

get_target_property(BOOST_PATH Boost::serialization LOCATION)
get_filename_component(BOOST_PATH "${BOOST_PATH}" DIRECTORY)

# Compute paths at configure time so they can be embedded as literal strings
# in the install script — avoiding the ${var} vs \${var} escaping trap where
# install-time-only variables expand to empty string at configure time.
set(_deploy_app    "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
set(_deploy_stamp  "${CMAKE_INSTALL_PREFIX}/.deps-stamp")
# Hash the build-tree binary (never touched by codesign) so the stamp remains
# valid across install runs even when codesign rewrites the deployed binary.
set(_build_binary  "${CMAKE_BINARY_DIR}/ui/app/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}")

INSTALL(CODE "
    # Re-run macdeployqt only when the build-tree binary has changed since the
    # last deploy. We key off the build-tree binary (not the deployed one) so
    # that codesign rewriting the deployed binary does not invalidate the stamp.
    # Delete the stamp file to force a full re-deploy (e.g. after rebuilding Qt).
    set(_need_deploy TRUE)
    if(EXISTS \"${_deploy_stamp}\" AND EXISTS \"${_build_binary}\")
        file(SHA256 \"${_build_binary}\" _bin_hash)
        file(READ   \"${_deploy_stamp}\"  _stamp_hash)
        string(STRIP \"\${_stamp_hash}\" _stamp_hash)
        if(\"\${_stamp_hash}\" STREQUAL \"\${_bin_hash}\")
            set(_need_deploy FALSE)
        endif()
    endif()

    if(_need_deploy)
        message(\"-- Installing Qt frameworks and plugins...\")
        file(REMOVE_RECURSE
             \"${_deploy_app}/Contents/Frameworks\"
             \"${_deploy_app}/Contents/PlugIns\"
             \"${_deploy_app}/Contents/Resources/qml\")
        file(REMOVE \"${_deploy_app}/Contents/MacOS/workrave-core-shadow-helper\")
        file(MAKE_DIRECTORY \"${_deploy_app}/Contents/Frameworks\")
        execute_process(COMMAND ln -sf ./Frameworks \"${_deploy_app}/Contents/lib\")
        execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} \"${_deploy_app}\" -verbose=1
               -qmldir=${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/qml
               \"$<$<BOOL:${BOOST_PATH}>:-libpath=${BOOST_PATH}>\"
               \"$<$<BOOL:${FMT_PATH}>:-libpath=${FMT_PATH}>\"
               \"$<$<BOOL:${SPDLOG_PATH}>:-libpath=${SPDLOG_PATH}>\"
               \"$<$<BOOL:${WORKRAVE_SIGN_IDENTITY}>:-sign-for-notarization=${WORKRAVE_SIGN_IDENTITY}>\"
               ${NO_STRIP} -always-overwrite
               RESULT_VARIABLE _deploy_result)
        if(NOT \${_deploy_result} EQUAL 0)
            message(FATAL_ERROR \"macdeployqt failed with exit code \${_deploy_result}\")
        endif()
        file(SHA256 \"${_build_binary}\" _bin_hash)
        file(WRITE  \"${_deploy_stamp}\"  \"\${_bin_hash}\")
    else()
        message(\"-- Qt frameworks up to date, skipping macdeployqt.\")
    endif()
    ")

if (HAVE_CORE_SHADOW)
    INSTALL(PROGRAMS "$<TARGET_FILE:workrave-core-shadow-helper>" DESTINATION "${BINDIR}")
    INSTALL(CODE "
        set(_helper \"${_deploy_app}/Contents/MacOS/workrave-core-shadow-helper\")
        if(EXISTS \"\${_helper}\")
            execute_process(COMMAND otool -l \"\${_helper}\" OUTPUT_VARIABLE _helper_lc ERROR_QUIET)
            if(NOT \"\${_helper_lc}\" MATCHES \"@executable_path/../Frameworks\")
                execute_process(COMMAND install_name_tool -add_rpath \"@executable_path/../Frameworks\" \"\${_helper}\" ERROR_QUIET)
            endif()
        endif()
    ")
endif()

# cmake --install always copies a fresh binary from the build tree, which resets
# the binary to its build-time rpaths.  macdeployqt is gated by the stamp, so
# when Qt frameworks are unchanged macdeployqt is skipped and the binary is left
# with raw build-tree rpaths.  Fix this every install run — it is fast.
INSTALL(CODE "
    set(_bin \"${_deploy_app}/Contents/MacOS/${PROJECT_NAME}\")
    set(_fwdir \"${_deploy_app}/Contents/Frameworks\")

    # 1. Ensure @executable_path/../Frameworks is in the binary's rpath so that
    #    all @rpath/... load commands resolve to the bundled Frameworks directory.
    execute_process(COMMAND otool -l \"\${_bin}\" OUTPUT_VARIABLE _lc ERROR_QUIET)
    if(NOT \"\${_lc}\" MATCHES \"@executable_path/../Frameworks\")
        execute_process(COMMAND install_name_tool -add_rpath \"@executable_path/../Frameworks\" \"\${_bin}\" ERROR_QUIET)
    endif()

    # 2. Fix any bare (no @ prefix) dylib references whose library is already in
    #    Frameworks (copied by a prior macdeployqt run).
    execute_process(COMMAND otool -L \"\${_bin}\" OUTPUT_VARIABLE _libs ERROR_QUIET)
    string(REGEX MATCHALL \"[^\n]+\" _lib_lines \"\${_libs}\")
    foreach(_line \${_lib_lines})
        string(STRIP \"\${_line}\" _line)
        if(\"\${_line}\" MATCHES \"^([^/@][^/ ]+\\\\.dylib)\")
            set(_libname \"\${CMAKE_MATCH_1}\")
            if(EXISTS \"\${_fwdir}/\${_libname}\")
                message(\"-- Fixing bare reference: \${_libname}\")
                execute_process(
                    COMMAND install_name_tool -change \"\${_libname}\"
                            \"@executable_path/../Frameworks/\${_libname}\"
                            \"\${_bin}\"
                )
            endif()
        endif()
    endforeach()
")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    # cmake --install always copies a fresh binary from the build tree, so debug
    # entitlements must be re-applied on every install — after macdeployqt, which
    # would otherwise overwrite them.  Ad-hoc signing (-) needs no developer account
    # and is sufficient for lldb to attach via com.apple.security.get-task-allow.
    INSTALL(CODE "
        message(\"-- Applying debug entitlements (lldb attach)...\")
        execute_process(
            COMMAND codesign --force --sign - --entitlements \"${_debug_entitlements}\" \"${_deploy_app}\"
            RESULT_VARIABLE _sign_result)
        if(NOT \${_sign_result} EQUAL 0)
            message(WARNING \"codesign failed with exit code \${_sign_result}\")
        endif()
    ")
endif()
