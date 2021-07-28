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

include(FetchContent)

FetchContent_Declare(
  WindowsThemeAcrylic
  GIT_REPOSITORY https://github.com/B00merang-Project/Windows-10-Acrylic.git
  GIT_TAG        e93765ea66d5440a13fb7b04d1fa0d2562516ab0
  )

FetchContent_Declare(
  WindowsTheme
  GIT_REPOSITORY https://github.com/B00merang-Project/Windows-10.git
  GIT_TAG        b2ca25265d19843b618a956cd0ffa4aed2b80527
  )

FetchContent_Declare(
  WindowsArtwork
  GIT_REPOSITORY https://github.com/B00merang-Artwork/Windows-10.git
  GIT_TAG        9f199c6b7c6050f36d0d15aaa6e9f4847e8b9066
  )

FetchContent_Declare(
  AdwaitaIconTheme
  GIT_REPOSITORY https://gitlab.gnome.org/GNOME/adwaita-icon-theme.git
  GIT_TAG        40.1.1
  )

FetchContent_GetProperties(WindowsThemeAcrylic)
if(NOT windowsthemeacrylic_POPULATED)
  FetchContent_Populate(WindowsThemeAcrylic)
endif()

FetchContent_GetProperties(WindowsTheme)
if(NOT windowstheme_POPULATED)
  FetchContent_Populate(WindowsTheme)
endif()

FetchContent_GetProperties(WindowsArtwork)
if(NOT windowsartwork_POPULATED)
  FetchContent_Populate(WindowsArtwork)
endif()

FetchContent_GetProperties(AdwaitaIconTheme)
if(NOT adwaitaicontheme_POPULATED)
  FetchContent_Populate(AdwaitaIconTheme)
endif()

install (DIRECTORY ${windowstheme_SOURCE_DIR}/gtk-3.20 DESTINATION share/themes/Windows10/)
install (DIRECTORY ${windowsthemeacrylic_SOURCE_DIR}/gtk-3.0 DESTINATION share/themes/Windows10Acrylic/)

if(MINGW) # TODO:
   install (DIRECTORY ${SYS_ROOT}/etc/gtk-3.0 DESTINATION etc)

   set (icons
    "legacy/go-first"
    "legacy/go-last"
    "legacy/go-next"
    "legacy/go-previous"
    "legacy/media-playback-pause"
    "legacy/media-playback-start"
    "legacy/media-skip-backward"
    "legacy/media-skip-forward"
    "legacy/window-close"
    "legacy/edit-delete"
    "legacy/edit-redo-rtl"
    "legacy/edit-redo"
    "legacy/application-exit"
    "legacy/preferences-other"
    "legacy/help-about"
    "legacy/document-open"
    "places/user-home-symbolic.symbolic"
    "ui/pan-down-symbolic.symbolic"
    "ui/pan-start-symbolic.symbolic"
    "ui/pan-up-symbolic.symbolic"
    "ui/pan-end-symbolic-rtl.symbolic"
    "ui/pan-start-symbolic-rtl.symbolic"
    "ui/pan-end-symbolic.symbolic"
    )
foreach(icon ${icons})
  foreach(size 16x16 24x24 32x32 48x48 )
     get_filename_component(category "${icon}" DIRECTORY)
     install(
       FILES ${SYS_ROOT}/share/icons/Adwaita/${size}/${icon}.png
       DESTINATION ${DATADIR}/icons/Adwaita/${size}/${category}
       )
  endforeach()
endforeach()

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/index.theme DESTINATION ${DATADIR}/icons/Adwaita)
install (FILES ${SYS_ROOT}/share/themes/Default/gtk-3.0/gtk-keys.css DESTINATION share/themes/Default/gtk-3.0)
install (DIRECTORY ${SYS_ROOT}/lib/gdk-pixbuf-2.0/2.10.0 DESTINATION lib/gdk-pixbuf-2.0/)
endif()

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/settings.ini DESTINATION etc/gtk-3.0)

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
  file(GLOB FILES32BIT ${PREBUILT_PATH}/lib32/*.*)
  install(FILES ${FILES32BIT} DESTINATION lib32)
elseif(HAVE_PREBUILT_VISUAL_STUDIO)
  install (FILES ${PREBUILT_PATH}/Release/harpoon.dll DESTINATION lib32)
  install (FILES ${PREBUILT_PATH}/Release64/harpoon64.dll DESTINATION lib)
  install (FILES ${PREBUILT_PATH}/Release64/harpoonHelper.exe DESTINATION lib32 RENAME WorkraveHelper.exe)
  install (FILES ${PREBUILT_PATH}/Release/workrave-applet.dll DESTINATION lib32)
  install (FILES ${PREBUILT_PATH}/Release64/workrave-applet64.dll DESTINATION lib)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

install(CODE "set(WINE ${WINE})")
install(CODE "set(ISCC ${ISCC})")
install(CODE "set(SYS_ROOT ${SYS_ROOT})")
install(CODE "set(ISCC ${ISCC})")
install(CODE "set(CMAKE_OBJDUMP ${CMAKE_OBJDUMP})")
install(CODE "set(MODULE_PATH ${CMAKE_SOURCE_DIR}/build/cmake/modules)")
install(CODE "set(DIST_PATH ${CMAKE_CURRENT_BINARY_DIR})")

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Installer.cmake")
