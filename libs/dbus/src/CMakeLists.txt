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
    target_include_directories(workrave-libs-dbus PUBLIC ${GLIB_INCLUDE_DIRS})
endif()

if (HAVE_DBUS_QT)
  target_sources(workrave-libs-dbus PRIVATE
    DBusQt.cc
    DBusGeneric.cc
    DBusBindingQt.cc)
    target_link_libraries(workrave-libs-dbus PUBLIC Qt${QT_VERSION_MAJOR}::DBus)
endif()

target_link_libraries(workrave-libs-dbus PRIVATE workrave-libs-utils Boost::boost)
target_include_directories(workrave-libs-dbus PUBLIC ${CMAKE_SOURCE_DIR}/libs/dbus/include)

if (HAVE_DBUS_TEST_GIO)
  set_target_properties(workrave-libs-dbus PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_GIO=1")
endif()

add_library(workrave-libs-dbus-stub STATIC
  DBusFactory.cc
  DBusException.cc
  DBusDummy.cc)

if (HAVE_DBUS_QT)
  target_sources(workrave-libs-dbus-stub PRIVATE DBusBindingQt.cc)
  target_link_libraries(workrave-libs-dbus-stub PRIVATE Qt${QT_VERSION_MAJOR}::DBus)
endif()

if (HAVE_DBUS_GIO)
  target_sources(workrave-libs-dbus-stub PRIVATE DBusBindingGio.cc)
  target_link_libraries(workrave-libs-dbus-stub PRIVATE ${GLIB_LIBRARIES})
  target_link_directories(workrave-libs-dbus-stub PRIVATE ${GLIB_LIBRARY_DIRS})
  target_include_directories(workrave-libs-dbus-stub PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

set_target_properties(workrave-libs-dbus-stub PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_DUMMY=1")
target_link_libraries(workrave-libs-dbus-stub PRIVATE workrave-libs-utils Boost::boost)
target_include_directories(workrave-libs-dbus-stub PUBLIC ${CMAKE_SOURCE_DIR}/libs/dbus/include)
