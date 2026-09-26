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
#   DESTDIR=<dir> ninja install/strip
#   ninja appimage

set(APPIMAGE_APPDIR "${CMAKE_BINARY_DIR}/AppDir" CACHE PATH
    "Root of the DESTDIR-staged, --prefix=/usr install tree the appimage target packages")

set(APPIMAGE_VERSION "" CACHE STRING
    "Version string linuxdeploy embeds in the AppImage metadata (optional)")

set(_appimage_target_arch "${CMAKE_SYSTEM_PROCESSOR}")
set(_appimage_host_arch "${CMAKE_HOST_SYSTEM_PROCESSOR}")
foreach(_arch_var _appimage_target_arch _appimage_host_arch)
  if("${${_arch_var}}" MATCHES "^(arm64|aarch64)$")
    set(${_arch_var} aarch64)
  elseif("${${_arch_var}}" MATCHES "^(amd64|AMD64|x86_64)$")
    set(${_arch_var} x86_64)
  endif()
endforeach()
set(APPIMAGE_TARGET_ARCH "${_appimage_target_arch}" CACHE STRING "Architecture of the AppImage payload")
if(NOT APPIMAGE_TARGET_ARCH STREQUAL _appimage_target_arch)
  message(FATAL_ERROR "APPIMAGE_TARGET_ARCH=${APPIMAGE_TARGET_ARCH} does not match the compiler target ${_appimage_target_arch}; use a cross toolchain")
endif()

# Packaging programs execute on the host, even when the payload is ARM64.
set(APPIMAGE_TOOLS_DIR "${CMAKE_SOURCE_DIR}/_ext/appimage-${_appimage_host_arch}" CACHE PATH
    "Where linuxdeploy and its gtk plugin are downloaded/cached")

set(LINUXDEPLOY "${APPIMAGE_TOOLS_DIR}/linuxdeploy-${_appimage_host_arch}.AppImage")
set(LINUXDEPLOY_PLUGIN_GTK "${APPIMAGE_TOOLS_DIR}/linuxdeploy-plugin-gtk.sh")
set(APPIMAGE_RUNTIME "${APPIMAGE_TOOLS_DIR}/runtime-${APPIMAGE_TARGET_ARCH}")
set(APPIMAGE_OUTPUT_FILE "${CMAKE_BINARY_DIR}/Workrave-${APPIMAGE_TARGET_ARCH}.AppImage")

# linuxdeploy and its gtk plugin are only needed to actually produce an
# AppImage, so they're fetched as a build step of the `appimage` target
# below (DownloadAppImageTools.cmake) rather than here at configure time.
# That keeps a plain configure/`ninja` (all) of a GTK Linux build from
# requiring network access or these packaging-only tools.

# linuxdeploy looks for its plugins on PATH.
get_filename_component(APPIMAGE_TOOLS_DIR_ABS "${APPIMAGE_TOOLS_DIR}" ABSOLUTE)

set(_appimage_cross_env)
if(CMAKE_CROSSCOMPILING)
  # linuxdeploy bundles a host-only strip and ignores $STRIP. The caller
  # strips our binaries using install/strip; Ubuntu's libraries are already
  # stripped. Keep the bundled strip away from target binaries.
  list(APPEND _appimage_cross_env "NO_STRIP=1")
endif()

add_custom_command(OUTPUT "${APPIMAGE_OUTPUT_FILE}"
                   COMMAND "${CMAKE_COMMAND}"
                           "-DAPPIMAGE_TOOLS_DIR=${APPIMAGE_TOOLS_DIR}"
                           "-DAPPIMAGE_HOST_ARCH=${_appimage_host_arch}"
                           "-DAPPIMAGE_TARGET_ARCH=${APPIMAGE_TARGET_ARCH}"
                           "-DAPPIMAGE_RUNTIME=${APPIMAGE_RUNTIME}"
                           "-DLINUXDEPLOY=${LINUXDEPLOY}"
                           "-DLINUXDEPLOY_PLUGIN_GTK=${LINUXDEPLOY_PLUGIN_GTK}"
                           -P "${CMAKE_CURRENT_SOURCE_DIR}/DownloadAppImageTools.cmake"
                   COMMAND "${CMAKE_COMMAND}" -E env
                           "PATH=${APPIMAGE_TOOLS_DIR_ABS}:$ENV{PATH}"
                           "LD_LIBRARY_PATH=${APPIMAGE_APPDIR}/usr/lib:${APPIMAGE_APPDIR}/usr/lib/${APPIMAGE_TARGET_ARCH}-linux-gnu"
                           "APPIMAGE_EXTRACT_AND_RUN=1"
                           "ARCH=${APPIMAGE_TARGET_ARCH}"
                           "LDAI_RUNTIME_FILE=${APPIMAGE_RUNTIME}"
                           "LDAI_OUTPUT=${APPIMAGE_OUTPUT_FILE}"
                           ${_appimage_cross_env}
                           "VERSION=${APPIMAGE_VERSION}"
                           "${LINUXDEPLOY}"
                           --appdir "${APPIMAGE_APPDIR}"
                           --plugin gtk
                           --output appimage
                           --icon-file "${APPIMAGE_APPDIR}/usr/share/icons/hicolor/scalable/apps/workrave.svg"
                           --desktop-file "${APPIMAGE_APPDIR}/usr/share/applications/org.workrave.Workrave.desktop"
                   WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                   DEPENDS workrave
                   VERBATIM)

add_custom_target(appimage DEPENDS "${APPIMAGE_OUTPUT_FILE}")
