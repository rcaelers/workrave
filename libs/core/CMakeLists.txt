if (NOT HAVE_CORE_NEXT OR HAVE_CORE_SHADOW)
  add_subdirectory(src)
  add_subdirectory(test)
endif()
