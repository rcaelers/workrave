add_subdirectory(applets)
add_subdirectory(common)
add_subdirectory(data)

if (HAVE_APP_GTK)
  add_subdirectory(gtkmm)
endif()

if (HAVE_APP_QT5)
  add_subdirectory(qt5)
endif()
