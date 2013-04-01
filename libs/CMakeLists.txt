add_subdirectory(utils)
add_subdirectory(config)
add_subdirectory(input-monitor)

if(HAVE_DBUS)
  add_subdirectory(dbus)
endif()
