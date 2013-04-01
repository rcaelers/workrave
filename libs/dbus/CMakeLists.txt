set(SRC
  src/DBus-gio.cc
  src/DBusBinding-gio.cc
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-dbus STATIC ${SRC})
