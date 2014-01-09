if (UNIX AND NOT APPLE)

  set(SRC
    timebar.c
    timerbox.c
    )
  
  if (HAVE_GTK)
    include_directories(${GTK_INCLUDE_DIRS})
  endif()
  
  include_directories(
    ${CMAKE_SOURCE_DIR}/frontend/applets/common/include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    ${CMAKE_SOURCE_DIR}/libs/config/include
    )
  
  add_library(workrave-private SHARED ${SRC})
  link_directories(${GTK_LIBPATH})
  target_link_libraries(workrave-private ${GTK_LIBRARIES})
  
  install(TARGETS workrave-private RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX} ARCHIVE DESTINATION lib${LIB_SUFFIX})
  set_target_properties(workrave-private PROPERTIES VERSION 1.0 SOVERSION 1.0)
  
  macro(_list_prefix _outvar _listvar _prefix)
    set(${_outvar})
    foreach(_item IN LISTS ${_listvar})
      list(APPEND ${_outvar} ${_prefix}${_item})
    endforeach()
  endmacro(_list_prefix)
  
  # GObject Introspection
  if (HAVE_INTROSPECTION)
    include(GObjectIntrospectionMacros)
    set(INTROSPECTION_GIRS)
    set(INTROSPECTION_SCANNER_ARGS --add-include-path=${CMAKE_CURRENT_SOURCE_DIR} --add-include-path=${CMAKE_CURRENT_SOURCE_DIR}/include --warn-all)
    set(INTROSPECTION_COMPILER_ARGS --includedir=${CMAKE_CURRENT_SOURCE_DIR} --includedir=${CMAKE_CURRENT_SOURCE_DIR}/include)
    
    set(introspection_files ${SRC} ../include/timerbox.h ../include/timebar.h)
    set(Workrave_1_0_gir "workrave-private")
    set(Workrave_1_0_gir_INCLUDES GObject-2.0 Gtk-3.0 cairo-1.0)
    set(Workrave_1_0_gir_CFLAGS ${GTK_CFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(Workrave_1_0_gir_LIBS workrave-private)
    set(Workrave_1_0_gir_VERSION "1.0")
    _list_prefix(_abs_introspection_files introspection_files "${CMAKE_CURRENT_SOURCE_DIR}/")
    set(Workrave_1_0_gir_FILES ${_abs_introspection_files})
    set(Workrave_1_0_gir_EXPORT_PACKAGES workrave-private)
    
    list(APPEND INTROSPECTION_GIRS Workrave-1.0.gir)
    
    gir_add_introspections(INTROSPECTION_GIRS)
  endif ()
endif()
