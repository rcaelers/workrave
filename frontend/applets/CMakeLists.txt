if (HAVE_GTK)
add_subdirectory(common)
endif()

if (HAVE_GNOME3)
  add_subdirectory(gnome3)
endif()

if (HAVE_INDICATOR)
  add_subdirectory(indicator)
endif()

add_subdirectory(gnome-shell)
