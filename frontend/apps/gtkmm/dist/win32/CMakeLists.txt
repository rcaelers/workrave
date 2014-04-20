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

  if ("${WITH_UI}" STREQUAL "Gtk+2")
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk20-properties.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk20.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  elseif ("${WITH_UI}" STREQUAL "Gtk+2") 
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30-properties.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  endif()
  
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/atk10.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gdk-pixbuf.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/glib20.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  elseif(EXISTS /usr/share/locale/${language}/LC_MESSAGES/iso_3166.mo)
    install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
    # TODO: else: give warning, once
  endif()

  if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
  elseif(EXISTS /usr/share/locale/${language}/LC_MESSAGES/iso_639.mo)
    install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION dist/lib/locale/${language}/LC_MESSAGES)
    # TODO: else: give warning, once
  endif()
endforeach()

# TODO: also build the prebuilt
# TODO: support Gtk+2 and Gtk+3

if ("${WITH_UI}" STREQUAL "Gtk+2")
  # Skip Gio for now. not needed.
  #install (DIRECTORY ${SYS_ROOT}/lib/gio/modules DESTINATION dist/lib/gio FILES_MATCHING PATTERN "*.dll")
  install (DIRECTORY ${SYS_ROOT}/etc/fonts DESTINATION dist/etc)
  install (DIRECTORY ${SYS_ROOT}/etc/gtk-2.0 DESTINATION dist/etc)
  install (DIRECTORY ${SYS_ROOT}/lib/gtk-2.0/2.10.0/engines DESTINATION dist/lib/gtk-2.0/2.10.0/ FILES_MATCHING PATTERN "*.dll")
  install (DIRECTORY ${SYS_ROOT}/share/themes DESTINATION dist/share)
  
elseif ("${WITH_UI}" STREQUAL "Gtk+2") 
  install (DIRECTORY ${SYS_ROOT}/etc/fonts DESTINATION dist/etc)
  install (DIRECTORY ${SYS_ROOT}/etc/gtk-3.0 DESTINATION dist/etc)
  install (DIRECTORY ${SYS_ROOT}/share/themes DESTINATION dist/share)
 
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateInstaller.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake"
  @ONLY
  ESCAPE_QUOTES
)

string(REPLACE "/" "\\" PREBUILTDIR ${PREBUILT_PATH})
string(REPLACE "/" "\\" INSTALLDIR ${CMAKE_INSTALL_PREFIX})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake")