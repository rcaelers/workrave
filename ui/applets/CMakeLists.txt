add_subdirectory(common)

add_subdirectory(cinnamon)

if (HAVE_GNOME45)
  add_subdirectory(gnome-shell-45)
else()
  add_subdirectory(gnome-shell)
endif()

if (HAVE_GNOME3_PANEL_APPLET)
  add_subdirectory(gnome3)
endif()

if (HAVE_INDICATOR)
  add_subdirectory(indicator)
endif()

if (HAVE_MATE)
  add_subdirectory(mate)
endif()

if (WIN32)
  add_subdirectory(windows)
endif()

if (HAVE_XFCE4)
  add_subdirectory(xfce)
endif()
