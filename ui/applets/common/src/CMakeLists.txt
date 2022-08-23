set(SRC
  control.c
  timebar.c
  timerbox.c
  utils.c
  )

if (HAVE_GTK AND UNIX AND NOT APPLE)

  add_library(workrave-private-1.0 SHARED ${SRC})

  target_include_directories(workrave-private-1.0
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/applets/common/include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    ${CMAKE_SOURCE_DIR}/libs/config/include
    ${GTK3_INCLUDE_DIRS}
    INTERFACE
    ${CMAKE_SOURCE_DIR}/ui/applets/common/include
    )

    target_link_directories(workrave-private-1.0 PRIVATE ${GTK3_LIBPATH})
    target_link_libraries(workrave-private-1.0 ${GTK3_LIBRARIES})

  set_target_properties(workrave-private-1.0 PROPERTIES VERSION 0.0.0 SOVERSION 0)
  install(TARGETS workrave-private-1.0 RUNTIME DESTINATION bin LIBRARY DESTINATION ${LIBDIR} ARCHIVE DESTINATION ${LIBDIR} LIBRARY NAMELINK_SKIP)

  macro(_list_prefix _outvar _listvar _prefix)
    set(${_outvar})
    foreach(_item IN LISTS ${_listvar})
      list(APPEND ${_outvar} ${_prefix}${_item})
    endforeach()
  endmacro()

  # GObject Introspection
  if (HAVE_INTROSPECTION)
    include(GObjectIntrospectionMacros)
    set(INTROSPECTION_GIRS)
    set(INTROSPECTION_SCANNER_ARGS --add-include-path=${CMAKE_CURRENT_SOURCE_DIR} --add-include-path=${CMAKE_CURRENT_SOURCE_DIR}/include --warn-all)
    set(INTROSPECTION_COMPILER_ARGS --includedir=${CMAKE_CURRENT_SOURCE_DIR} --includedir=${CMAKE_CURRENT_SOURCE_DIR}/include)

    string(REGEX REPLACE "-std=[a-zA-Z0-9+]+" "" GTK_CFLAGS_FILTERED ${GTK_CFLAGS})

    set(introspection_files ${SRC} ../include/timerbox.h ../include/timebar.h)
    set(Workrave_1_0_gir "workrave-private")
    set(Workrave_1_0_gir_INCLUDES GObject-2.0 Gtk-3.0 cairo-1.0)
    set(Workrave_1_0_gir_CFLAGS ${GTK_CFLAGS_FILTERED} -I${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(Workrave_1_0_gir_LIBS workrave-private-1.0)
    set(Workrave_1_0_gir_VERSION "1.0")
    _list_prefix(_abs_introspection_files introspection_files "${CMAKE_CURRENT_SOURCE_DIR}/")
    set(Workrave_1_0_gir_FILES ${_abs_introspection_files})
    set(Workrave_1_0_gir_EXPORT_PACKAGES workrave-private)

    list(APPEND INTROSPECTION_GIRS Workrave-1.0.gir)

    gir_add_introspections(INTROSPECTION_GIRS)
  endif ()
endif()
