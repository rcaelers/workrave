set(SRC
  src/zapper.c
  )

add_library(zapper-0 SHARED ${SRC})
install(TARGETS zapper-0 RUNTIME DESTINATION dist)

install (FILES WizModernImage.bmp DESTINATION dist)
install (FILES WizModernSmall.bmp DESTINATION dist)

file(GLOB POFILES ${CMAKE_SOURCE_DIR}/po/*.po)

foreach (po_file ${POFILES})
  get_filename_component(language ${po_file} NAME_WE)

  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/atk10.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gdk-pixbuf.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/glib20.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk20-properties.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk20.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
endforeach()

# TODO: use objdump
# TODO: support Gtk+2 and Gtk+3
install (FILES ${SYS_ROOT}/bin/libatkmm-1.6-1.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libboost_filesystem-mt.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libboost_system-mt.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgdk-win32-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgdk_pixbuf-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgdkmm-2.4-1.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgio-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libglib-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libglibmm-2.4-1.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgobject-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgtk-win32-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgtkmm-2.4-1.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libintl-8.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libpango-1.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libpangomm-1.4-1.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libsigc-2.0-0.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libgcc_s_sjlj-1.dll DESTINATION dist/lib)
install (FILES ${SYS_ROOT}/bin/libstdc++-6.dll DESTINATION dist/lib)

install (DIRECTORY ${SYS_ROOT}/lib/gio/modules DESTINATION dist/lib/gio FILES_MATCHING PATTERN "*.dll")
install (DIRECTORY ${SYS_ROOT}/etc/fonts DESTINATION dist/etc)
install (DIRECTORY ${SYS_ROOT}/etc/gtk-2.0 DESTINATION dist/etc)
install (DIRECTORY ${SYS_ROOT}/lib/gtk-2.0/2.10.0/engines DESTINATION dist/lib/gtk-2.0/2.10.0/ FILES_MATCHING PATTERN "*.dll")
install (DIRECTORY ${SYS_ROOT}/share/themes DESTINATION dist/share)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateInstaller.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake"
  @ONLY
  ESCAPE_QUOTES
)

# TODO: also build the prebuilt
string(REPLACE "/" "\\" PREBUILTDIR ${WORKRAVE_PREBUILT})
string(REPLACE "/" "\\" INSTALLDIR ${CMAKE_INSTALL_PREFIX})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake")