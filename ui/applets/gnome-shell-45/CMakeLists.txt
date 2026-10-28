add_subdirectory(src)

if (HAVE_TESTS)
  find_program(NODE_EXECUTABLE NAMES node nodejs)
  if (NODE_EXECUTABLE)
    add_test(NAME workrave-gnome-shell-menu-test
      COMMAND ${NODE_EXECUTABLE} --test ${CMAKE_CURRENT_SOURCE_DIR}/test/menu.test.cjs)
  endif()
endif()
