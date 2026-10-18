# Copyright (C) 2010, Pino Toscano, <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(_gir_list_prefix _outvar _listvar _prefix)
  set(${_outvar})
  foreach(_item IN LISTS ${_listvar})
    list(APPEND ${_outvar} ${_prefix}${_item})
  endforeach()
endmacro(_gir_list_prefix)

macro(gir_add_introspections introspections_girs)

  set(_gir_girs)
  set(_gir_typelibs)

  foreach(gir IN LISTS ${introspections_girs})

    set(_gir_name "${gir}")

    ## Transform the gir filename to something which can reference through a variable
    ## without automake/make complaining, eg Gtk-2.0.gir -> Gtk_2_0_gir
    string(REPLACE "-" "_" _gir_name "${_gir_name}")
    string(REPLACE "." "_" _gir_name "${_gir_name}")

    # Namespace and Version is either fetched from the gir filename
    # or the _NAMESPACE/_VERSION variable combo
    set(_gir_namespace "${${_gir_name}_NAMESPACE}")
    if (_gir_namespace STREQUAL "")
      string(REGEX REPLACE "([^-]+)-.*" "\\1" _gir_namespace "${gir}")
    endif ()
    set(_gir_version "${${_gir_name}_VERSION}")
    if (_gir_version STREQUAL "")
      string(REGEX REPLACE ".*-([^-]+).gir" "\\1" _gir_version "${gir}")
    endif ()

    # _PROGRAM is an optional variable which needs it's own --program argument
    set(_gir_program "${${_gir_name}_PROGRAM}")
    if (NOT _gir_program STREQUAL "")
      set(_gir_program "--program=${_gir_program}")
    endif ()

    # Variables which provides a list of things
    _gir_list_prefix(_gir_libraries ${_gir_name}_LIBS "--library=")
    _gir_list_prefix(_gir_packages ${_gir_name}_PACKAGES "--pkg=")
    _gir_list_prefix(_gir_includes ${_gir_name}_INCLUDES "--include=")
    set(_gir_library_paths)
    foreach(_gir_library IN LISTS ${_gir_name}_LIBS)
      if(TARGET ${_gir_library})
        list(APPEND _gir_library_paths "--library-path=$<TARGET_FILE_DIR:${_gir_library}>")
      endif()
    endforeach()

    # Reuse the LIBTOOL variable from by automake if it's set
    set(_gir_libtool "--no-libtool")

    # The scanner compiles, links, and runs a helper outside CMake. Use our
    # compiler even for native builds: mixing GCC and Clang sanitizer runtimes
    # does not work. Cross builds retain their scanner wrapper/QEMU runner.
    set(_gir_cc "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}")
    if(CMAKE_C_COMPILER_TARGET)
      string(APPEND _gir_cc " --target=${CMAKE_C_COMPILER_TARGET}")
    endif()
    if(CMAKE_SYSROOT)
      string(APPEND _gir_cc " --sysroot=\"${CMAKE_SYSROOT}\"")
    endif()
    set(_gir_cflags "${CMAKE_C_FLAGS} ${_workrave_sanitizer_link_flags}")
    set(_gir_ldflags "${CMAKE_EXE_LINKER_FLAGS} ${_workrave_sanitizer_link_flags}")
    set(_gir_configs ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
    list(REMOVE_DUPLICATES _gir_configs)
    foreach(_gir_config IN LISTS _gir_configs)
      string(TOUPPER "${_gir_config}" _gir_upper_config)
      string(APPEND _gir_cflags " $<$<CONFIG:${_gir_config}>:${CMAKE_C_FLAGS_${_gir_upper_config}}>")
      string(APPEND _gir_ldflags " $<$<CONFIG:${_gir_config}>:${CMAKE_EXE_LINKER_FLAGS_${_gir_upper_config}}>")
    endforeach()
    set(_gir_scanner_command ${CMAKE_COMMAND} -E env
        "CC=${_gir_cc}" "CFLAGS=${_gir_cflags}" "LDFLAGS=${_gir_ldflags}"
        ${INTROSPECTION_SCANNER})

    add_custom_command(
      COMMAND ${_gir_scanner_command}
              ${INTROSPECTION_SCANNER_ARGS}
              --namespace=${_gir_namespace}
              --nsversion=${_gir_version}
              ${_gir_libtool}
              ${_gir_program}
              ${_gir_libraries}
              ${_gir_library_paths}
              ${_gir_packages}
              ${_gir_includes}
              ${${_gir_name}_SCANNERFLAGS}
              ${${_gir_name}_CFLAGS}
              ${${_gir_name}_FILES}
              --output ${CMAKE_CURRENT_BINARY_DIR}/${gir}
      DEPENDS ${${_gir_name}_FILES}
              ${${_gir_name}_LIBS}
      OUTPUT ${gir}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      VERBATIM
    )
    list(APPEND _gir_girs ${CMAKE_CURRENT_BINARY_DIR}/${gir})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${gir} DESTINATION share/gir-1.0)

    string(REPLACE ".gir" ".typelib" _typelib "${gir}")
    add_custom_command(
      COMMAND ${INTROSPECTION_COMPILER}
              ${INTROSPECTION_COMPILER_ARGS}
              --includedir=.
              ${CMAKE_CURRENT_BINARY_DIR}/${gir}
              -o ${CMAKE_CURRENT_BINARY_DIR}/${_typelib}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${gir}
      OUTPUT ${_typelib}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    list(APPEND _gir_typelibs ${CMAKE_CURRENT_BINARY_DIR}/${_typelib})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_typelib} DESTINATION ${LIBDIR}/girepository-1.0)

  endforeach()

  add_custom_target(gir-girs ALL DEPENDS ${_gir_girs})
  add_custom_target(gir-typelibs ALL DEPENDS ${_gir_typelibs})

endmacro(gir_add_introspections)
