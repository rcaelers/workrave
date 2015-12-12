if (HAVE_DBUS AND HAVE_QT5 AND HAVE_TESTS)

  dbus_generate_with_backend(DBusTestQt ${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusTestQt.cc qt5)

  set(SERVER_QT5_SRC ${SERVER_QT5_SRC}
    DBusTestData.cc
    DBusTestServer.cc
    DBusTestServerQt5.cc
    DBusTestQt.cc
    )

  qt5_wrap_cpp(MOC_SRC
    Test.hh
  )

  set(TEST_SRC ${TEST_SRC} ${MOC_SRC}
    DBusTestData.cc
    Test.cc
    )

  add_executable(workrave-libs-dbus-test ${TEST_SRC})
  add_cppcheck(workrave-libs-dbus-test)
  add_executable(workrave-libs-dbus-test-server-qt5 ${SERVER_QT5_SRC})
  add_cppcheck(workrave-libs-dbus-test-server-qt5)
  set_target_properties(workrave-libs-dbus-test-server-qt5 PROPERTIES COMPILE_DEFINITIONS "DBUS_BACKEND_QT5=1")
  set_target_properties(workrave-libs-dbus-test PROPERTIES COMPILE_DEFINITIONS "DBUS_BACKEND_QT5=1")

  include_directories(
    ${CMAKE_SOURCE_DIR}/src/libs/dbus/src
    ${CMAKE_SOURCE_DIR}/src/libs/dbus/include
    ${CMAKE_SOURCE_DIR}/src/libs/utils/include
    )

  if (HAVE_DBUS_FREEDESKTOP)
    target_link_libraries(workrave-libs-dbus-test-server-qt5 ${DBUS_LIBRARIES})
  endif()

  target_link_libraries(workrave-libs-dbus-test workrave-libs-dbus)
  target_link_libraries(workrave-libs-dbus-test workrave-libs-utils)
  target_link_libraries(workrave-libs-dbus-test ${Qt5DBus_LIBRARIES})
  target_link_libraries(workrave-libs-dbus-test ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

  target_link_libraries(workrave-libs-dbus-test-server-qt5 workrave-libs-dbus)
  target_link_libraries(workrave-libs-dbus-test-server-qt5 workrave-libs-utils)
  target_link_libraries(workrave-libs-dbus-test-server-qt5 ${Qt5DBus_LIBRARIES})
  target_link_libraries(workrave-libs-dbus-test-server-qt5 ${Boost_LIBRARIES})

  if (HAVE_DBUS_TEST_GIO)
    dbus_generate_with_backend(DBusTestGio ${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusTestGio.cc gio)

    set(SERVER_GIO_SRC ${SERVER_GIO_SRC}
      DBusTestData.cc
      DBusTestServer.cc
      DBusTestServerGio.cc
      DBusTestGio.cc
      )

    add_executable(workrave-libs-dbus-test-server-gio ${SERVER_GIO_SRC})
    add_cppcheck(workrave-libs-dbus-test-server-gio)

    set_target_properties(workrave-libs-dbus-test-server-gio PROPERTIES COMPILE_DEFINITIONS "DBUS_BACKEND_GIO=1")
    set_target_properties(workrave-libs-dbus-test-server-gio PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_GIO=1")
    include_directories(${GLIB_INCLUDE_DIRS})

    target_link_libraries(workrave-libs-dbus-test-server-gio workrave-libs-dbus)
    target_link_libraries(workrave-libs-dbus-test-server-gio workrave-libs-utils)
    target_link_libraries(workrave-libs-dbus-test-server-gio ${GLIB_LIBRARIES})
    target_link_libraries(workrave-libs-dbus-test-server-gio ${Boost_LIBRARIES})
  endif()
endif()
