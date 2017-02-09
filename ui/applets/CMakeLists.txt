if (GTK3_FOUND)
  add_subdirectory(common)
endif()

add_subdirectory(cinnamon)

add_subdirectory(gnome-shell)

if (HAVE_PANELAPPLET)
  add_subdirectory(gnome3)
endif()

if (HAVE_INDICATOR)
  add_subdirectory(indicator)
endif()

if (HAVE_MATE)
  add_subdirectory(mate)
endif()

if (WIN32)
  add_subdirectory(win32)
endif()

if (HAVE_XFCE4)
  add_subdirectory(xfce)
endif()
