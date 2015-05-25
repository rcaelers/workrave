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
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk20-properties.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk20.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  elseif ("${WITH_UI}" STREQUAL "Gtk+2") 
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30-properties.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  endif()
  
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/atk10.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gdk-pixbuf.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/glib20.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  elseif(EXISTS /usr/share/locale/${language}/LC_MESSAGES/iso_3166.mo)
    install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
    # TODO: else: give warning, once
  endif()

  if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  elseif(EXISTS /usr/share/locale/${language}/LC_MESSAGES/iso_639.mo)
    install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
    # TODO: else: give warning, once
  endif()
endforeach()

# TODO: also build the prebuilt
# TODO: support Gtk+2 and Gtk+3

if ("${WITH_UI}" STREQUAL "Gtk+2")
  # Skip Gio for now. not needed.
  # install (DIRECTORY ${SYS_ROOT}/lib/gio/modules DESTINATION lib/gio FILES_MATCHING PATTERN "*.dll")
  install (DIRECTORY ${SYS_ROOT}/etc/gtk-2.0 DESTINATION etc)
  install (DIRECTORY ${SYS_ROOT}/lib/gtk-2.0/2.10.0/engines DESTINATION lib/gtk-2.0/2.10.0/ FILES_MATCHING PATTERN "*.dll")
  install (DIRECTORY ${SYS_ROOT}/share/themes DESTINATION share)
  install (FILES ${SYS_ROOT}/share/themes/MS-Windows/gtk-2.0/gtkrc DESTINATION share/themes/Raleigh/gtk-2.0/)
  
elseif ("${WITH_UI}" STREQUAL "Gtk+3") 
  install (DIRECTORY ${SYS_ROOT}/etc/gtk-3.0 DESTINATION etc)
  install (FILES ${SYS_ROOT}/share/themes/Default/gtk-3.0/gtk-keys.css DESTINATION share/themes/Default/gtk-3.0)

  set(SYSTEM_DEFAULT_THEME ${CMAKE_INSTALL_PREFIX}/share/icons/Adwaita)
  install (DIRECTORY ${SYS_ROOT}/share/icons DESTINATION share)

  install(
        CODE 
        "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/etc/gtk-3.0/settings.ini\" \"[Settings]\\ngtk-theme-name=gtk-win32\\n\")"
        CODE
        "execute_process(COMMAND gtk-update-icon-cache -t -f ${SYSTEM_DEFAULT_THEME})"
  )

endif()

install (DIRECTORY ${SYS_ROOT}/etc/pango DESTINATION etc)
install (DIRECTORY ${SYS_ROOT}/etc/fonts DESTINATION etc)
install (DIRECTORY ${SYS_ROOT}/lib/pango/1.8.0/modules DESTINATION lib/pango/1.8.0/modules FILES_MATCHING PATTERN "*.dll")
install (DIRECTORY ${SYS_ROOT}/share/glib-2.0/schemas/ DESTINATION share/glib-2.0/schemas/ FILES_MATCHING PATTERN "org.gtk*.xml")
install (FILES ${SYS_ROOT}/share/glib-2.0/schemas/gschema.dtd DESTINATION share/glib-2.0/schemas/)
install(CODE "execute_process (COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas\")")

install (FILES ${HOOK_BINARY_DIR}/Release/harpoon.dll DESTINATION lib)
install (FILES ${HOOK_BINARY_DIR}/Release64/harpoon64.dll DESTINATION lib)
install (FILES ${HOOK_BINARY_DIR}/Release64/harpoonHelper.exe DESTINATION lib)
install (FILES ${APPLET_BINARY_DIR}/Release/workrave-applet.dll DESTINATION lib)
install (FILES ${APPLET_BINARY_DIR}/Release64/workrave-applet.dll DESTINATION lib RENAME workrave-applet64.dll)


#close open redo media (previous next pause play) preferences about quiet goto_first goto_last go_back go_forward delete

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateInstaller.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake"
  @ONLY
  ESCAPE_QUOTES
)

string(REPLACE "/" "\\" INSTALLDIR ${CMAKE_INSTALL_PREFIX})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake")
