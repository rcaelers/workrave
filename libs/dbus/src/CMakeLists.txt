add_library(workrave-libs-dbus STATIC
  DBusFactory.cc
  DBusException.cc
  DBusDummy.cc
)

if (HAVE_DBUS_GIO OR HAVE_DBUS_TEST_GIO)
  target_sources(workrave-libs-dbus PRIVATE 
    DBusGio.cc
    DBusGeneric.cc
    DBusBindingGio.cc)
    target_include_directories(workrave-libs-dbus PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

if (HAVE_DBUS_FREEDESKTOP)
  target_sources(workrave-libs-dbus PRIVATE 
    DBusFreedesktop.cc
    DBusBindingFreedesktop.cc)
  target_link_libraries(workrave-libs-dbus ${DBUS_LIBRARIES})
endif()

if (HAVE_DBUS_QT5)
  target_sources(workrave-libs-dbus PRIVATE 
    DBusQt5.cc
    DBusGeneric.cc
    DBusBindingQt5.cc)
  target_include_directories(workrave-libs-dbus PUBLIC ${Qt5DBus_INCLUDE_DIRS})
endif()

target_link_libraries(workrave-libs-dbus workrave-libs-utils)
target_include_directories(workrave-libs-dbus PUBLIC ${CMAKE_SOURCE_DIR}/libs/dbus/include)

if (HAVE_DBUS_TEST_GIO)
  set_target_properties(workrave-libs-dbus PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_GIO=1")
endif()

add_library(workrave-libs-dbus-stub STATIC
  DBusFactory.cc
  DBusException.cc
  DBusDummy.cc)

if (HAVE_DBUS_QT5)
  target_sources(workrave-libs-dbus-stub PRIVATE DBusBindingQt5.cc)
  target_include_directories(workrave-libs-dbus-stub PRIVATE ${Qt5DBus_INCLUDE_DIRS})
endif()
if (HAVE_DBUS_GIO)
  target_sources(workrave-libs-dbus-stub PRIVATE DBusBindingGio.cc)
endif()

set_target_properties(workrave-libs-dbus-stub PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_DUMMY=1")
target_link_libraries(workrave-libs-dbus-stub workrave-libs-utils)
target_include_directories(workrave-libs-dbus-stub PUBLIC ${CMAKE_SOURCE_DIR}/libs/dbus/include)
