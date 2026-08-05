configure_file(${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/dist/macos/Info.plist.in ${CMAKE_BINARY_DIR}/Info.plist)

list(APPEND CPACK_GENERATOR "DragNDrop")

set(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns)

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
# Stamp is keyed on the macdeployqt executable (a reliable Qt-version proxy).
# macdeployqt only needs to re-run when Qt itself changes, not on every binary
# relink.  Binary-only updates are handled by the fast rpath-fixup block below.
# Delete the stamp to force a full re-deploy (e.g. after switching Qt versions).
set(_deploy_stamp  "${CMAKE_INSTALL_PREFIX}/.deps-stamp")

INSTALL(CODE "
    set(_need_deploy TRUE)
    if(EXISTS \"${_deploy_stamp}\")
        file(SHA256 \"${MACDEPLOYQT_EXECUTABLE}\" _mdt_hash)
        file(READ   \"${_deploy_stamp}\"            _stamp_hash)
        string(STRIP \"\${_stamp_hash}\" _stamp_hash)
        if(\"\${_stamp_hash}\" STREQUAL \"\${_mdt_hash}\")
            set(_need_deploy FALSE)
        endif()
    endif()

    if(_need_deploy)
        message(\"-- Installing Qt frameworks and plugins...\")
        file(REMOVE_RECURSE
             \"${_deploy_app}/Contents/Frameworks\"
             \"${_deploy_app}/Contents/PlugIns\"
             \"${_deploy_app}/Contents/Resources/qml\"
	     \"${_deploy_app}/Contents/Resources/lib\"
	     \"${_deploy_app}/Contents/lib\"
	     )
        file(REMOVE \"${_deploy_app}/Contents/MacOS/workrave-core-shadow-helper\")
        file(MAKE_DIRECTORY \"${_deploy_app}/Contents/Frameworks\")
        execute_process(COMMAND ${MACDEPLOYQT_EXECUTABLE} \"${_deploy_app}\" -verbose=1
               -qmldir=${CMAKE_SOURCE_DIR}/ui/app/toolkits/qt/qml
               \"$<$<BOOL:${BOOST_PATH}>:-libpath=${BOOST_PATH}>\"
               \"$<$<BOOL:${FMT_PATH}>:-libpath=${FMT_PATH}>\"
               \"$<$<BOOL:${SPDLOG_PATH}>:-libpath=${SPDLOG_PATH}>\"
               \"$<$<BOOL:${WORKRAVE_SIGN_IDENTITY}>:-sign-for-notarization=${WORKRAVE_SIGN_IDENTITY}>\"
               ${NO_STRIP}
               RESULT_VARIABLE _deploy_result)
        if(NOT \${_deploy_result} EQUAL 0)
            message(FATAL_ERROR \"macdeployqt failed with exit code \${_deploy_result}\")
        endif()
        # Create the lib -> Frameworks alias only *after* macdeployqt has run.
        # Creating it beforehand made macdeployqt walk into the alias while
        # resolving dependencies and embed a stray self-referential
        # Contents/Frameworks/Frameworks -> ./Frameworks symlink, which later
        # made codesign/notarization fail with ELOOP (\"too many levels of
        # symbolic links\").
        execute_process(COMMAND ln -sf ./Frameworks \"${_deploy_app}/Contents/lib\")
	    if(NOT \"${WORKRAVE_SIGN_IDENTITY}\")
            set(_app ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app)
            message(\"-- Ad-hoc signing app bundle...\")
            execute_process(COMMAND codesign --force --deep --sign - \${_app})
	    endif()
        file(SHA256 \"${MACDEPLOYQT_EXECUTABLE}\" _mdt_hash)
        file(WRITE  \"${_deploy_stamp}\"            \"\${_mdt_hash}\")
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

# Sign ${PROJECT_NAME}.app with a Developer ID identity and submit it to Apple's
# notary service. Deliberately kept out of "all" / the default install — it needs
# a Developer ID identity and notarytool credentials that most builds don't have,
# and it talks to Apple's servers, so it must be invoked explicitly:
#   cmake --build . --target notarize
#
# One-time setup:
#   -DWORKRAVE_SIGN_IDENTITY="Developer ID Application: ..."   (see `security find-identity -v -p codesigning`)
#   xcrun notarytool store-credentials <profile> --apple-id <apple-id> --team-id <team-id> --password <app-specific-password>
#   -DWORKRAVE_NOTARIZE_PROFILE=<profile>
set(WORKRAVE_NOTARIZE_PROFILE "" CACHE STRING "xcrun notarytool keychain profile used to notarize ${PROJECT_NAME}.app")

add_custom_target(notarize
    # The install step below only re-runs macdeployqt (and therefore only signs
    # the bundled Qt frameworks/plugins with WORKRAVE_SIGN_IDENTITY) when this
    # stamp is missing or stale — see the deploy-stamp logic above. Force it on
    # every notarize run so a previous ad-hoc-signed (or unsigned) deploy never
    # leaves nested frameworks without a proper Developer ID + secure timestamp.
    COMMAND ${CMAKE_COMMAND} -E rm -f "${_deploy_stamp}"
    COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND}
            "-DWORKRAVE_APP=${_deploy_app}"
            "-DWORKRAVE_SIGN_IDENTITY=${WORKRAVE_SIGN_IDENTITY}"
            "-DWORKRAVE_NOTARIZE_PROFILE=${WORKRAVE_NOTARIZE_PROFILE}"
            "-DWORKRAVE_NOTARIZE_ZIP=${CMAKE_BINARY_DIR}/${PROJECT_NAME}-notarize.zip"
            "-DWORKRAVE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/notarize.cmake"
    DEPENDS workrave
    COMMENT "Signing and notarizing ${PROJECT_NAME}.app with Apple (not part of 'all')"
    VERBATIM)

# Packages ${PROJECT_NAME}.app into a disk image ready to distribute. Also kept
# out of "all":
#   cmake --build . --target dmg
#
# Deliberately does not use CPack's DragNDrop generator ("bundle" target,
# ui/app/toolkits/qt/dist/CMakeLists.txt) — CPack re-installs into its own
# staging directory from scratch, which would package an unsigned/unnotarized
# copy instead of the bundle that "notarize" actually signed and stapled. See
# make-dmg.cmake for the details.
#
# With WORKRAVE_SIGN_IDENTITY configured, this depends on "notarize" so the
# packaged app is always freshly signed, notarized and stapled, and
# make-dmg.cmake additionally signs/notarizes/staples the dmg itself. Without
# it (e.g. signing intentionally disabled for a local test build), there's no
# Developer ID to notarize with, so this instead just installs the (ad-hoc
# signed, per the plain install rule above) app and make-dmg.cmake skips
# straight to packaging it — still a fully usable, double-clickable dmg for
# local testing, just not something to distribute.
if(WORKRAVE_SIGN_IDENTITY)
    add_custom_target(dmg
        COMMAND ${CMAKE_COMMAND}
                "-DWORKRAVE_APP=${_deploy_app}"
                "-DWORKRAVE_DMG_STAGING=${CMAKE_BINARY_DIR}/dmg-staging"
                "-DWORKRAVE_DMG_OUTPUT=${CMAKE_INSTALL_PREFIX}/${CPACK_PACKAGE_FILE_NAME}.dmg"
                "-DWORKRAVE_DMG_BACKGROUND=${CMAKE_CURRENT_SOURCE_DIR}/dmg_background.png"
                "-DWORKRAVE_DMG_ICON=${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns"
                "-DWORKRAVE_DMG_APPLESCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/dmg.applescript"
                "-DWORKRAVE_VOLUME_NAME=${PROJECT_NAME}"
                "-DWORKRAVE_SIGN_IDENTITY=${WORKRAVE_SIGN_IDENTITY}"
                "-DWORKRAVE_NOTARIZE_PROFILE=${WORKRAVE_NOTARIZE_PROFILE}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/make-dmg.cmake"
        DEPENDS notarize
        COMMENT "Building a signed, notarized disk image of ${PROJECT_NAME}.app (not part of 'all')"
        VERBATIM)
else()
    add_custom_target(dmg
        COMMAND ${CMAKE_COMMAND} -E rm -f "${_deploy_stamp}"
        COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND}
                "-DWORKRAVE_APP=${_deploy_app}"
                "-DWORKRAVE_DMG_STAGING=${CMAKE_BINARY_DIR}/dmg-staging"
                "-DWORKRAVE_DMG_OUTPUT=${CMAKE_INSTALL_PREFIX}/${CPACK_PACKAGE_FILE_NAME}.dmg"
                "-DWORKRAVE_DMG_BACKGROUND=${CMAKE_CURRENT_SOURCE_DIR}/dmg_background.png"
                "-DWORKRAVE_DMG_ICON=${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns"
                "-DWORKRAVE_DMG_APPLESCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/dmg.applescript"
                "-DWORKRAVE_VOLUME_NAME=${PROJECT_NAME}"
                "-DWORKRAVE_SIGN_IDENTITY="
                "-DWORKRAVE_NOTARIZE_PROFILE="
                -P "${CMAKE_CURRENT_SOURCE_DIR}/make-dmg.cmake"
        DEPENDS workrave
        COMMENT "Building an unsigned disk image of ${PROJECT_NAME}.app for local testing (not part of 'all')"
        VERBATIM)
endif()
