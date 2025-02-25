install(FILES WizModernImage.bmp DESTINATION dist)
install(FILES WizModernSmall.bmp DESTINATION dist)

if(MINGW OR VCPKG_TOOLCHAIN)
  file(GLOB POFILES ${CMAKE_SOURCE_DIR}/po/*.po)
  foreach (po_file ${POFILES})
    get_filename_component(language ${po_file} NAME_WE)

    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30-properties.mo DESTINATION share/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30.mo DESTINATION share/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gdk-pixbuf.mo DESTINATION share/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/glib20.mo DESTINATION share/locale/${language}/LC_MESSAGES)

    if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo)
      install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION share/locale/${language}/LC_MESSAGES)
    elseif ()
      install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION share/locale/${language}/LC_MESSAGES)
      # TODO: else: give warning, once
    endif()

    if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo)
      install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION share/locale/${language}/LC_MESSAGES)
    elseif()
      install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION share/locale/${language}/LC_MESSAGES)
      # TODO: else: give warning, once
    endif()
  endforeach()
endif()

include(FetchContent)

FetchContent_Declare_Logged(
  WindowsThemeAcrylic
  "GTK theme based on the Windows 10 Mobile appearance"
  "GPL-3.0"
  GIT_REPOSITORY https://github.com/B00merang-Project/Windows-10-Acrylic.git
  GIT_TAG        e93765ea66d5440a13fb7b04d1fa0d2562516ab0
  )

  FetchContent_Declare_Logged(
  WindowsTheme
  "Windows 10 Light GTK theme"
  "GPL-3.0"
  GIT_REPOSITORY https://github.com/B00merang-Project/Windows-10.git
  GIT_TAG        b2ca25265d19843b618a956cd0ffa4aed2b80527
  )

  FetchContent_Declare_Logged(
  WindowsArtwork
  "Windows 10 Icon theme"
  "Unknown"
  GIT_REPOSITORY https://github.com/B00merang-Artwork/Windows-10.git
  GIT_TAG        9f199c6b7c6050f36d0d15aaa6e9f4847e8b9066
  )

FetchContent_GetProperties(WindowsThemeAcrylic)
if(NOT windowsthemeacrylic_POPULATED)
  FetchContent_MakeAvailable(WindowsThemeAcrylic)
endif()

FetchContent_GetProperties(WindowsTheme)
if(NOT windowstheme_POPULATED)
  FetchContent_MakeAvailable(WindowsTheme)
endif()

FetchContent_GetProperties(WindowsArtwork)
if(NOT windowsartwork_POPULATED)
  FetchContent_MakeAvailable(WindowsArtwork)
endif()

install (DIRECTORY ${windowstheme_SOURCE_DIR}/gtk-3.20 DESTINATION share/themes/Windows10/)
install (DIRECTORY ${windowsthemeacrylic_SOURCE_DIR}/gtk-3.0 DESTINATION share/themes/Windows10Acrylic/)

if(MINGW OR VCPKG_TOOLCHAIN)
  install (DIRECTORY ${SYS_ROOT}/etc/gtk-3.0 DESTINATION etc)
  install (DIRECTORY ${SYS_ROOT}/lib/gdk-pixbuf-2.0/2.10.0 DESTINATION lib/gdk-pixbuf-2.0/ FILES_MATCHING PATTERN "*.dll" PATTERN "*.cache")

  install (DIRECTORY ${SYS_ROOT}/lib/pango/1.8.0/modules DESTINATION lib/pango/1.8.0/modules FILES_MATCHING PATTERN "*.dll")
  install (DIRECTORY ${SYS_ROOT}/etc/fonts DESTINATION etc)
  install (DIRECTORY ${SYS_ROOT}/share/glib-2.0/schemas/ DESTINATION share/glib-2.0/schemas/ FILES_MATCHING PATTERN "org.gtk*.xml")
  install (FILES ${SYS_ROOT}/share/glib-2.0/schemas/gschema.dtd DESTINATION share/glib-2.0/schemas/)
  install (CODE "execute_process (COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas\")")

  if (VCPKG_TOOLCHAIN)
    set(ADWAITA_DIR ${SYS_ROOT}/share/adwaita-icon-theme/icons/Adwaita)
  else()
    set(ADWAITA_DIR ${SYS_ROOT}/share/icons/Adwaita)
  endif()

  foreach(size 16x16 24x24 32x32 48x48 64x64 96x96 128x128 scalable)
    foreach(icon places/user* places/folder* places/drive*)
      file(GLOB globs RELATIVE "${ADWAITA_DIR}" "${ADWAITA_DIR}/${size}/${icon}.*")
      foreach(icon_file ${globs})
        get_filename_component(dir ${icon_file} DIRECTORY)
        install(FILES ${ADWAITA_DIR}/${icon_file} DESTINATION ${DATADIR}/icons/Adwaita/${dir})
      endforeach()
    endforeach()
  endforeach()

  install(FILES "${ADWAITA_DIR}/index.theme" DESTINATION ${DATADIR}/icons/Adwaita)
  install(CODE "execute_process(COMMAND gtk-update-icon-cache \${DATADIR}/icons/Adwaita)")
endif()

if (VCPKG_TOOLCHAIN)
  install(FILES ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/glib/gdbus.exe DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  install(FILES ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/glib/gspawn-win64-helper.exe DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  install(FILES ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/glib/gspawn-win64-helper-console.exe DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
else()
  install(FILES ${SYS_ROOT}/bin/gdbus.exe DESTINATION "bin")
  install(FILES ${SYS_ROOT}/bin/gspawn-win64-helper.exe DESTINATION "bin")
  install(FILES ${SYS_ROOT}/bin/gspawn-win64-helper-console.exe DESTINATION "bin")
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/settings.ini DESTINATION etc/gtk-3.0)

install(CODE "set(SYS_ROOT ${SYS_ROOT})")
install(CODE "set(MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)")
install(CODE "set(BINARY_DIR \"${CMAKE_BINARY_DIR}\")")
install(CODE "set(DIST_PATH \"${CMAKE_CURRENT_BINARY_DIR}\")")
install(CODE "set(INSTALL_PATH \"${CMAKE_INSTALL_PREFIX}\")")
install(CODE "set(DEP_DIRS \"${SYS_ROOT}/bin;${spdlog_BINARY_DIR};${CMAKE_INSTALL_PREFIX}/bin\")")
install(CODE "set(BINDIR \"${BINDIR}\")")
install(CODE "set(BINDIR32 \"${BINDIR32}\")")
install(CODE "set(HAVE_CRASHPAD \"${HAVE_CRASHPAD}\")")
install(CODE "set(HAVE_DBUS \"${HAVE_DBUS}\")")
install(CODE "set(HAVE_SBOM \"${HAVE_SBOM}\")")

if (MSVC)
  install(CODE "set(MSVC ${MSVC})")
  install(CODE "set(VCPKG_INSTALLED_DIR \"${VCPKG_INSTALLED_DIR}\")")
  install(CODE "set(VCPKG_TARGET_TRIPLET \"${VCPKG_TARGET_TRIPLET}\")")
  install(CODE "set(CONFIG ${CONFIG})")
  install(CODE "set(DEP_DIRS \"${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>bin\")")
endif()

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/InstallDependencies.cmake")
install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/PrepareInnoSetup.cmake")

string(REPLACE "/" "\\" INSTALL_WIN_PATH "${CMAKE_INSTALL_PREFIX}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

set(INSTALLER_NAME "workrave-installer")
set(INSTALLER_TARGET "${INSTALLER_NAME}.exe")
set(ISCC_FILENAME_FLAGS "/F${INSTALLER_NAME}")

if(WINE)
  add_custom_command(OUTPUT ${INSTALLER_TARGET}
                     COMMAND "${WINE}" "${ISCC}" setup.iss "/o${CMAKE_INSTALL_PREFIX}" "/DCHANNEL=${WORKRAVE_CHANNEL}" ${ISCC_FLAGS} ${ISCC_FILENAME_FLAGS}
                     DEPENDS workrave)
else()
  add_custom_command(OUTPUT ${INSTALLER_TARGET}
                     COMMAND "${ISCC}" setup.iss "/o${CMAKE_INSTALL_PREFIX}" "/DCHANNEL=${WORKRAVE_CHANNEL}" ${ISCC_FLAGS} ${ISCC_FILENAME_FLAGS}
                     DEPENDS workrave
                     VERBATIM)
endif()

add_custom_target(installer DEPENDS ${INSTALLER_TARGET})

if (CMAKE_CROSSCOMPILING)
  ExternalProject_Add(Zapper32Bit
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/.32
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchains/mingw32-gcc.cmake
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
  )
elseif (MSVC)
  ExternalProject_Add(Zapper32Bit
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/.32
    CMAKE_GENERATOR "Visual Studio 17 2022"
    CMAKE_ARGS -A Win32
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
      -DVCPKG_TARGET_TRIPLET:STRING=x86-windows
  )
elseif (MINGW)
  set(CMAKE_COMMAND32 ${MSYS_CMD} -here -mingw32 -no-start -defterm -c "cmake \$*" cmake)
  ExternalProject_Add(Zapper32Bit
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/.32
    CMAKE_COMMAND ${CMAKE_COMMAND32}
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchains/msys2.cmake
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
  )
endif()
