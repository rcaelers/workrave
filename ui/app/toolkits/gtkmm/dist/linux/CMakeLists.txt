# AppImage packaging.
#
# The AppImage payload must be built with CMAKE_INSTALL_PREFIX=/usr (so the
# absolute paths workrave compiles in, e.g. WORKRAVE_PKGDATADIR, resolve
# correctly once the AppImage remaps itself to look like /usr at runtime),
# staged into a private directory via DESTDIR rather than actually installed
# to /usr. Because CMAKE_INSTALL_PREFIX affects compiled-in definitions, that
# choice has to be made at the initial `cmake` configure step, so unlike the
# Windows installer/portable targets, this target does NOT drive its own
# install — the caller is expected to have already run, with a matching
# DESTDIR:
#
#   cmake <src> -DCMAKE_INSTALL_PREFIX=/usr -DAPPIMAGE_APPDIR=<dir>
#   DESTDIR=<dir> ninja install
#   ninja appimage

set(APPIMAGE_APPDIR "${CMAKE_BINARY_DIR}/AppDir" CACHE PATH
    "Root of the DESTDIR-staged, --prefix=/usr install tree the appimage target packages")

set(APPIMAGE_VERSION "" CACHE STRING
    "Version string linuxdeploy embeds in the produced AppImage's filename (optional)")

set(APPIMAGE_TOOLS_DIR "${CMAKE_SOURCE_DIR}/_ext/appimage-${CMAKE_SYSTEM_PROCESSOR}" CACHE PATH
    "Where linuxdeploy and its gtk plugin are downloaded/cached")

set(LINUXDEPLOY "${APPIMAGE_TOOLS_DIR}/linuxdeploy-${CMAKE_SYSTEM_PROCESSOR}.AppImage")
set(LINUXDEPLOY_PLUGIN_GTK "${APPIMAGE_TOOLS_DIR}/linuxdeploy-plugin-gtk.sh")

# linuxdeploy and its gtk plugin are only needed to actually produce an
# AppImage, so they're fetched as a build step of the `appimage` target
# below (DownloadAppImageTools.cmake) rather than here at configure time.
# That keeps a plain configure/`ninja` (all) of a GTK Linux build from
# requiring network access or these packaging-only tools.

# linuxdeploy looks for its plugins on PATH.
get_filename_component(APPIMAGE_TOOLS_DIR_ABS "${APPIMAGE_TOOLS_DIR}" ABSOLUTE)

# linuxdeploy names its own output (Workrave-<version>-<arch>.AppImage) and
# there's no way to know that name at configure time, so track completion
# with a stamp file instead of the AppImage path itself.
set(APPIMAGE_STAMP "${CMAKE_BINARY_DIR}/appimage.stamp")

add_custom_command(OUTPUT "${APPIMAGE_STAMP}"
                   COMMAND "${CMAKE_COMMAND}"
                           "-DAPPIMAGE_TOOLS_DIR=${APPIMAGE_TOOLS_DIR}"
                           "-DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}"
                           "-DLINUXDEPLOY=${LINUXDEPLOY}"
                           "-DLINUXDEPLOY_PLUGIN_GTK=${LINUXDEPLOY_PLUGIN_GTK}"
                           -P "${CMAKE_CURRENT_SOURCE_DIR}/DownloadAppImageTools.cmake"
                   COMMAND "${CMAKE_COMMAND}" -E env
                           "PATH=${APPIMAGE_TOOLS_DIR_ABS}:$ENV{PATH}"
                           "LD_LIBRARY_PATH=${APPIMAGE_APPDIR}/usr/lib:${APPIMAGE_APPDIR}/usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu"
                           "VERSION=${APPIMAGE_VERSION}"
                           "${LINUXDEPLOY}"
                           --appdir "${APPIMAGE_APPDIR}"
                           --plugin gtk
                           --output appimage
                           --icon-file "${APPIMAGE_APPDIR}/usr/share/icons/hicolor/scalable/apps/workrave.svg"
                           --desktop-file "${APPIMAGE_APPDIR}/usr/share/applications/org.workrave.Workrave.desktop"
                   COMMAND "${CMAKE_COMMAND}" -E touch "${APPIMAGE_STAMP}"
                   WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                   DEPENDS workrave
                   VERBATIM)

add_custom_target(appimage DEPENDS "${APPIMAGE_STAMP}")
