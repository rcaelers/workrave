if (HAVE_DBUS AND HAVE_QT AND HAVE_TESTS)

  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR} DBusTestQt qt)
  dbus_generate_xml(${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR} DBusTestQt qt)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR} DBusTestGio gio)

  set(SERVER_QT_SRC ${SERVER_QT_SRC}
    DBusTestData.cc
    DBusTestServer.cc
    DBusTestServerQt.cc
    DBusTestQt.cc
    )

  qt_wrap_cpp(MOC_SRC
    Test.hh
  )

  set(TEST_SRC ${TEST_SRC} ${MOC_SRC}
    DBusTestData.cc
    Test.cc
    )

  add_executable(workrave-libs-dbus-test ${TEST_SRC})
  add_executable(workrave-libs-dbus-test-server-qt ${SERVER_QT_SRC})
  set_target_properties(workrave-libs-dbus-test-server-qt PROPERTIES COMPILE_DEFINITIONS "DBUS_BACKEND_QT=1")
  set_target_properties(workrave-libs-dbus-test PROPERTIES COMPILE_DEFINITIONS "DBUS_BACKEND_QT=1")

  target_include_directories(workrave-libs-dbus-test PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/dbus/src
    ${CMAKE_SOURCE_DIR}/libs/dbus/include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    )

  target_include_directories(workrave-libs-dbus-test-server-qt
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libs/dbus/src
    ${CMAKE_SOURCE_DIR}/libs/dbus/include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    )

  target_link_libraries(workrave-libs-dbus-test PRIVATE workrave-libs-dbus)
  target_link_libraries(workrave-libs-dbus-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-libs-dbus-test PRIVATE Qt${QT_VERSION_MAJOR}::DBus)
  target_link_libraries(workrave-libs-dbus-test PRIVATE Boost::unit_test_framework)
  target_link_libraries(workrave-libs-dbus-test PRIVATE Boost::test_exec_monitor)
  target_link_libraries(workrave-libs-dbus-test PRIVATE ${EXTRA_LIBRARIES})

  target_link_libraries(workrave-libs-dbus-test-server-qt PRIVATE workrave-libs-dbus)
  target_link_libraries(workrave-libs-dbus-test-server-qt PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-libs-dbus-test-server-qt PRIVATE Qt${QT_VERSION_MAJOR}::DBus)
  target_link_libraries(workrave-libs-dbus-test-server-qt PRIVATE Boost::unit_test_framework)
  target_link_libraries(workrave-libs-dbus-test-server-qt PRIVATE Boost::test_exec_monitor)
  target_link_libraries(workrave-libs-dbus-test-server-qt PRIVATE ${EXTRA_LIBRARIES})

  if (HAVE_DBUS_TEST_GIO)
    set(SERVER_GIO_SRC ${SERVER_GIO_SRC}
      DBusTestData.cc
      DBusTestServer.cc
      DBusTestServerGio.cc
      DBusTestGio.cc
      )

    add_executable(workrave-libs-dbus-test-server-gio ${SERVER_GIO_SRC})

    set_target_properties(workrave-libs-dbus-test-server-gio PROPERTIES COMPILE_DEFINITIONS "DBUS_BACKEND_GIO=1")
    set_target_properties(workrave-libs-dbus-test-server-gio PROPERTIES COMPILE_DEFINITIONS "HAVE_DBUS_GIO=1")
    target_include_directories(workrave-libs-dbus-test-server-gio PRIVATE ${GLIB_INCLUDE_DIRS})

    target_link_libraries(workrave-libs-dbus-test-server-gio workrave-libs-dbus)
    target_link_libraries(workrave-libs-dbus-test-server-gio workrave-libs-utils)
    target_link_libraries(workrave-libs-dbus-test-server-gio ${GLIB_LIBRARIES})
    target_link_directories(workrave-libs-dbus-test-server-gio PRIVATE ${GLIB_LIBRARY_DIRS})
    target_link_libraries(workrave-libs-dbus-test-server-gio Boost::unit_test_framework)
    target_link_libraries(workrave-libs-dbus-test-server-gio Boost::test_exec_monitor)
    target_link_libraries(workrave-libs-dbus-test-server-gio ${EXTRA_LIBRARIES})

  endif()
endif()
