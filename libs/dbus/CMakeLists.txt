if (HAVE_DBUS_GIO)
  set(SRC ${SRC}
    src/DBus-gio.cc
    src/DBusBinding-gio.cc)
endif()

if (HAVE_DBUS_FREEDESKTOP)
  set(SRC ${SRC}
    src/DBus-freedesktop.cc
    src/DBusBinding-freedesktop.cc)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-dbus STATIC ${SRC})

if (HAVE_DBUS_FREEDESKTOP)
  target_link_libraries(workrave-libs-dbus ${DBUS_LIBRARIES})
endif()

