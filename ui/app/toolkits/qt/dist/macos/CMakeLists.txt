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

# CMake normally strips build-tree library paths during install. macdeployqt
# needs those paths to resolve @rpath dependencies before bundling them. This
# also works with Conan's INTERFACE facade targets, which have no LOCATION.
set_property(TARGET workrave PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

# QML plugins can depend on Qt modules that the executable does not link.
# Split Qt installations may expose them through the shared library directory
# reported by qtpaths, rather than the linked modules' individual prefixes.
# Put this before CMake's inferred link paths so macdeployqt carries the common
# directory into its recursive QML/plugin dependency scan. The post-deployment
# cleanup removes this path along with the other build-machine rpaths.
set_property(TARGET workrave APPEND PROPERTY BUILD_RPATH "${QT_LIB_DIR}")
set_property(TARGET workrave APPEND PROPERTY INSTALL_RPATH "${QT_LIB_DIR}")

set(_deploy_app "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
set(_deploy_stamp "${CMAKE_INSTALL_PREFIX}/.deps-stamp")
set(_deploy_shadow_helper "")
if(HAVE_CORE_SHADOW)
    set(_deploy_shadow_helper "$<TARGET_FILE:workrave-core-shadow-helper>")
endif()

# Configure paths once, then resolve the optional helper target per build config.
# Keep install-time logic in an ordinary script instead of escaped install(CODE).
configure_file(deploy.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/deploy.cmake.in" @ONLY)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/deploy-$<CONFIG>.cmake"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/deploy.cmake.in")
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/deploy-$<CONFIG>.cmake")

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
