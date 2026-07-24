set(SRC
  control.c
  timebar.c
  timerbox.c
  utils.c
  )

if (UNIX AND NOT APPLE)

  if (HAVE_GTK3)
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
  endif()

  if (HAVE_GTK4)
    add_library(workrave-gtk4-private-1.0 SHARED ${SRC})

    target_include_directories(workrave-gtk4-private-1.0
      PRIVATE
      ${CMAKE_SOURCE_DIR}/ui/applets/common/include
      ${CMAKE_SOURCE_DIR}/libs/utils/include
      ${CMAKE_SOURCE_DIR}/libs/config/include
      ${GTK4_INCLUDE_DIRS}
      INTERFACE
      ${CMAKE_SOURCE_DIR}/ui/applets/common/include
      )

    target_link_directories(workrave-gtk4-private-1.0 PRIVATE ${GTK4_LIBPATH})
    target_link_libraries(workrave-gtk4-private-1.0 ${GTK4_LIBRARIES})
    target_compile_definitions(workrave-gtk4-private-1.0 PRIVATE HAVE_GTK4)

    set_target_properties(workrave-gtk4-private-1.0 PROPERTIES VERSION 0.0.0 SOVERSION 0)
    install(TARGETS workrave-gtk4-private-1.0 RUNTIME DESTINATION bin LIBRARY DESTINATION ${LIBDIR} ARCHIVE DESTINATION ${LIBDIR} LIBRARY NAMELINK_SKIP)
  endif()

  macro(_list_prefix _outvar _listvar _prefix)
    set(${_outvar})
    foreach(_item IN LISTS ${_listvar})
      list(APPEND ${_outvar} ${_prefix}${_item})
    endforeach()
  endmacro()

  function(filter_cflags original_flags filtered_flags_var)
    set(filtered_flags "")
    foreach(flag IN LISTS original_flags)
        if(flag MATCHES "^-I" OR flag MATCHES "^-D")
            set(filtered_flags "${filtered_flags} ${flag}")
        endif()
    endforeach()
    set(${filtered_flags_var} "${filtered_flags}" PARENT_SCOPE)
  endfunction()

  # GObject Introspection
  if (HAVE_INTROSPECTION)
    include(GObjectIntrospectionMacros)
    set(INTROSPECTION_GIRS)
    set(INTROSPECTION_SCANNER_ARGS --add-include-path=${CMAKE_CURRENT_SOURCE_DIR} --add-include-path=${CMAKE_CURRENT_SOURCE_DIR}/include --warn-all)
    set(INTROSPECTION_COMPILER_ARGS --includedir=${CMAKE_CURRENT_SOURCE_DIR} --includedir=${CMAKE_CURRENT_SOURCE_DIR}/include)

    if (HAVE_GTK3)
      filter_cflags("${GTK_CFLAGS}" GTK_CFLAGS_FILTERED)

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
    endif()

    if (HAVE_GTK4)
      filter_cflags("${GTK4_CFLAGS}" GTK4_CFLAGS_FILTERED)

      set(introspection_files ${SRC} ../include/timerbox.h ../include/timebar.h)
      set(Workrave_2_0_gir "workrave-gtk4-private")
      set(Workrave_2_0_gir_INCLUDES GObject-2.0 Gtk-4.0 cairo-1.0)
      set(Workrave_2_0_gir_CFLAGS ${GTK4_CFLAGS_FILTERED} -I${CMAKE_CURRENT_SOURCE_DIR}/include)
      set(Workrave_2_0_gir_LIBS workrave-gtk4-private-1.0)
      set(Workrave_2_0_gir_VERSION "2.0")
      _list_prefix(_abs_introspection_files introspection_files "${CMAKE_CURRENT_SOURCE_DIR}/")
      set(Workrave_2_0_gir_FILES ${_abs_introspection_files})
      set(Workrave_2_0_gir_EXPORT_PACKAGES workrave-gtk4-private)

      list(APPEND INTROSPECTION_GIRS Workrave-2.0.gir)
    endif()

    gir_add_introspections(INTROSPECTION_GIRS)
  endif ()
endif()
