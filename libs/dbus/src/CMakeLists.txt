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

set(SRC ${SRC}
  DBusFactory.cc
  DBusException.cc
  DBusDummy.cc
)


include_directories(
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-dbus STATIC ${SRC})
add_cppcheck(workrave-libs-dbus)

if (HAVE_DBUS_FREEDESKTOP)
  target_link_libraries(workrave-libs-dbus ${DBUS_LIBRARIES})
endif()

if (HAVE_DBUS_TEST_GIO)
  set_target_properties(workrave-libs-dbus PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_GIO=1")
endif()

set(STUB_SRC ${STUB_SRC}
  DBusFactory.cc
  DBusException.cc
  DBusDummy.cc
)

if (HAVE_DBUS_QT5)
  set(STUB_SRC ${STUB_SRC}
    DBusBindingQt5.cc)
endif()
if (HAVE_DBUS_GIO)
  set(STUB_SRC ${STUB_SRC}
    DBusBindingGio.cc)
endif()

add_library(workrave-libs-dbus-stub STATIC ${STUB_SRC})
add_cppcheck(workrave-libs-dbus-stub)
set_target_properties(workrave-libs-dbus-stub PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_DUMMY=1")

