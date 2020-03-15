set(SRC src/zapper.c)

add_library(zapper-0 SHARED ${SRC})
if (MINGW)
   set_target_properties(zapper-0 PROPERTIES LINK_FLAGS "-static-libgcc")
endif()
if (MSVC)
  set_target_properties(zapper-0 PROPERTIES COMPILE_FLAGS "-DWIN32_LEAN_AND_MEAN")
endif()

install(TARGETS zapper-0 RUNTIME DESTINATION dist)

install (FILES WizModernImage.bmp DESTINATION dist)
install (FILES WizModernSmall.bmp DESTINATION dist)

string(REPLACE "/" "\\" INSTALLDIR ${CMAKE_INSTALL_PREFIX})

file(GLOB POFILES ${CMAKE_SOURCE_DIR}/po/*.po)

set(LANG_ISS "${CMAKE_CURRENT_BINARY_DIR}/lang.iss")
file(WRITE ${LANG_ISS} "; Translations\n")

foreach (po_file ${POFILES})
  get_filename_component(language ${po_file} NAME_WE)

  if(MINGW) # TODO:
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30-properties.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gtk30.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/atk10.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/gdk-pixbuf.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/glib20.mo DESTINATION lib/locale/${language}/LC_MESSAGES)

  if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  elseif ()
    install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_3166.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
    # TODO: else: give warning, once
  endif()

  if (EXISTS ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo)
    install (FILES ${SYS_ROOT}/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
  elseif()
    install (FILES /usr/share/locale/${language}/LC_MESSAGES/iso_639.mo DESTINATION lib/locale/${language}/LC_MESSAGES)
    # TODO: else: give warning, once
  endif()
  endif()

  file(APPEND ${LANG_ISS} "Source: \"${INSTALLDIR}\\lib\\locale\\${language}\\LC_MESSAGES\\*.*\"; DestDir: \"{app}\\lib\\locale\\${language}\\LC_MESSAGES\"; Flags: ignoreversion;\n")
endforeach()


install (FILES gtk.css gtk-win32-base.css DESTINATION share/themes/windows/gtk-3.0)
if(MINGW) # TODO:
   install (DIRECTORY ${SYS_ROOT}/etc/gtk-3.0 DESTINATION etc)

   set (icons
    "actions/go-"
    "actions/media-playback-"
    "actions/media-skip-"
    "actions/window-"
    "actions/edit-"
    "actions/application-exit"
    "actions/preferences"
    "actions/help-about"
    "actions/document-open"
    "actions/list-remove-symbolic"
    "actions/list-remove-symbolic.symbolic"
    "actions/list-add-symbolic"
    "actions/list-add-symbolic.symbolic"
    "places/user-home-symbolic.symbolic"
    "places/user-home-"
    )
foreach(icon ${icons})
  foreach(size 16x16 24x24 32x32 48x48 64x64 96x96 128x128 scalable)
    install(
      DIRECTORY ${SYS_ROOT}/share/icons/Adwaita/${size}/
      DESTINATION ${DATADIR}/icons/Adwaita/${size}/
      FILES_MATCHING
      REGEX "${icon}[a-zA-Z0-0]*\.(png|svg)"
      )
  endforeach()
endforeach()

install (FILES ${SYS_ROOT}/share/themes/Default/gtk-3.0/gtk-keys.css DESTINATION share/themes/Default/gtk-3.0)
install (DIRECTORY ${SYS_ROOT}/lib/gdk-pixbuf-2.0/2.10.0 DESTINATION lib/gdk-pixbuf-2.0/)

install (FILES ${SYS_ROOT}/bin/librsvg-2-2.dll DESTINATION lib)
endif()

install (CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/etc/gtk-3.0/settings.ini\" \"[Settings]\ngtk-theme-name=windows\n\")")

if(MINGW) # TODO:
if (NOT MSYS)
  install (DIRECTORY ${SYS_ROOT}/etc/pango DESTINATION etc)
endif()
install (DIRECTORY ${SYS_ROOT}/etc/fonts DESTINATION etc)
install (DIRECTORY ${SYS_ROOT}/lib/pango/1.8.0/modules DESTINATION lib/pango/1.8.0/modules FILES_MATCHING PATTERN "*.dll")
install (DIRECTORY ${SYS_ROOT}/share/glib-2.0/schemas/ DESTINATION share/glib-2.0/schemas/ FILES_MATCHING PATTERN "org.gtk*.xml")
install (FILES ${SYS_ROOT}/share/glib-2.0/schemas/gschema.dtd DESTINATION share/glib-2.0/schemas/)
install (CODE "execute_process (COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas\")")
endif()

if (HAVE_PREBUILT_CMAKE)
  file(GLOB FILES64BIT ${PREBUILT_PATH}/lib64/*.*)
  install(FILES ${FILES64BIT} DESTINATION lib64)
elseif(HAVE_PREBUILT_VISUAL_STUDIO)
  install (FILES ${PREBUILT_PATH}/Release/harpoon.dll DESTINATION lib)
  install (FILES ${PREBUILT_PATH}/Release64/harpoon64.dll DESTINATION lib64)
  install (FILES ${PREBUILT_PATH}/Release64/harpoonHelper.exe DESTINATION lib64 RENAME WorkraveHelper.exe)
  install (FILES ${PREBUILT_PATH}/Release/workrave-applet.dll DESTINATION lib)
  install (FILES ${PREBUILT_PATH}/Release64/workrave-applet64.dll DESTINATION lib64)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CreateInstaller.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake"
  @ONLY
  ESCAPE_QUOTES
)


configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CreateInstaller.cmake")
