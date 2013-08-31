add_subdirectory(applets)
add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(apps)

if (HAVE_APP_GTK)
  add_subdirectory(gtkmm)
endif()
