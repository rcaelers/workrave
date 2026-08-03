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

# These tools are generic (unrelated to this checkout's version), so caching
# them at configure time, keyed only by architecture, mirrors the FetchContent
# caching already used for the Windows GTK themes above.
if (NOT EXISTS "${LINUXDEPLOY}")
  file(MAKE_DIRECTORY "${APPIMAGE_TOOLS_DIR}")
  message(STATUS "Downloading linuxdeploy")
  file(DOWNLOAD
       "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${CMAKE_SYSTEM_PROCESSOR}.AppImage"
       "${LINUXDEPLOY}"
       STATUS _linuxdeploy_download_status)
  list(GET _linuxdeploy_download_status 0 _linuxdeploy_download_code)
  if (NOT _linuxdeploy_download_code EQUAL 0)
    file(REMOVE "${LINUXDEPLOY}")
    message(FATAL_ERROR "Failed to download linuxdeploy: ${_linuxdeploy_download_status}")
  endif()

  # linuxdeploy ships as an AppImage itself, which can't run inside the
  # containers that build AppImages. Zeroing its embedded AppImage magic
  # bytes makes it run as a plain extracted ELF instead. See:
  # https://github.com/AppImage/AppImageKit/issues/965#issuecomment-1333557171
  find_program(DD_CMD dd REQUIRED)
  execute_process(COMMAND "${DD_CMD}" if=/dev/zero bs=1 count=3 seek=8 conv=notrunc of=${LINUXDEPLOY}
                   RESULT_VARIABLE _dd_result)
  if (NOT _dd_result EQUAL 0)
    message(FATAL_ERROR "Failed to patch linuxdeploy's AppImage magic bytes")
  endif()

  file(CHMOD "${LINUXDEPLOY}" PERMISSIONS
       OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

if (NOT EXISTS "${LINUXDEPLOY_PLUGIN_GTK}")
  file(MAKE_DIRECTORY "${APPIMAGE_TOOLS_DIR}")
  message(STATUS "Downloading linuxdeploy-plugin-gtk")
  file(DOWNLOAD
       "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh"
       "${LINUXDEPLOY_PLUGIN_GTK}"
       STATUS _linuxdeploy_gtk_download_status)
  list(GET _linuxdeploy_gtk_download_status 0 _linuxdeploy_gtk_download_code)
  if (NOT _linuxdeploy_gtk_download_code EQUAL 0)
    file(REMOVE "${LINUXDEPLOY_PLUGIN_GTK}")
    message(FATAL_ERROR "Failed to download linuxdeploy-plugin-gtk: ${_linuxdeploy_gtk_download_status}")
  endif()

  file(CHMOD "${LINUXDEPLOY_PLUGIN_GTK}" PERMISSIONS
       OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

# linuxdeploy looks for its plugins on PATH.
get_filename_component(APPIMAGE_TOOLS_DIR_ABS "${APPIMAGE_TOOLS_DIR}" ABSOLUTE)

# linuxdeploy names its own output (Workrave-<version>-<arch>.AppImage) and
# there's no way to know that name at configure time, so track completion
# with a stamp file instead of the AppImage path itself.
set(APPIMAGE_STAMP "${CMAKE_BINARY_DIR}/appimage.stamp")

add_custom_command(OUTPUT "${APPIMAGE_STAMP}"
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
