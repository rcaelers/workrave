set(SRC ${SRC}
  IDBus.cc
  DBusDummy.cc
)

if (HAVE_DBUS_GIO OR HAVE_DBUS_TEST_GIO)
  set(SRC ${SRC}
    DBusGio.cc
    DBusGeneric.cc
    DBusBindingGio.cc)

  include_directories(${GLIB_INCLUDE_DIRS})
endif()

if (HAVE_DBUS_FREEDESKTOP)
  set(SRC ${SRC}
    DBusFreedesktop.cc
    DBusBindingFreedesktop.cc)
endif()

if (HAVE_DBUS_QT5)
  set(SRC ${SRC}
    DBusQt5.cc 
    DBusGeneric.cc
    DBusBindingQt5.cc)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-dbus STATIC ${SRC})

if (HAVE_DBUS_FREEDESKTOP)
  target_link_libraries(workrave-libs-dbus ${DBUS_LIBRARIES})
endif()

if (HAVE_DBUS_TEST_GIO)
  set_target_properties(workrave-libs-dbus PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_GIO=1")
endif()

set(STUB_SRC ${STUB_SRC}
  IDBus.cc
  DBusDummy.cc
)

if (HAVE_DBUS_QT5)
  set(STUB_SRC ${STUB_SRC}
    DBusBindingQt5.cc)
endif()


add_library(workrave-libs-dbus-stub STATIC ${STUB_SRC})
set_target_properties(workrave-libs-dbus-stub PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_DUMMY=1")

