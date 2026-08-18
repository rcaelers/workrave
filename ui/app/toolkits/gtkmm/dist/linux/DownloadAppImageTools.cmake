# Downloads linuxdeploy and its gtk plugin into APPIMAGE_TOOLS_DIR, if not
# already cached there. Run as a build step of the `appimage` target (via
# `cmake -P`) rather than at configure time, so a plain `cmake` configure /
# `ninja` (all) of a GTK Linux build never needs network access or these
# packaging-only tools -- only `ninja appimage` does.
#
# Expected -D arguments: APPIMAGE_TOOLS_DIR, CMAKE_SYSTEM_PROCESSOR,
# LINUXDEPLOY, LINUXDEPLOY_PLUGIN_GTK

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
