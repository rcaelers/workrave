vcpkg_download_distfile(ARCHIVE_PATH
    URLS "https://download.gnome.org/sources/adwaita-icon-theme/43/adwaita-icon-theme-43.tar.xz"
    FILENAME "adwaita-icon-theme-43.tar.xz"
    SHA512 fe0c186c2dbe87ccf2373bde1bc5ab658e8cd64bf0e5a3b9cd1117d5c1bf2ef5cc83b76b7fae54fde1566a07b572d8bb9441f437e44813338195e191dbb2a021)

vcpkg_extract_source_archive(SOURCE_PATH ARCHIVE ${ARCHIVE_PATH})

vcpkg_configure_make(
    SOURCE_PATH "${SOURCE_PATH}"
    AUTOCONFIG
    USE_WRAPPERS
  )
vcpkg_install_make()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(INSTALL "${SOURCE_PATH}/COPYING" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
