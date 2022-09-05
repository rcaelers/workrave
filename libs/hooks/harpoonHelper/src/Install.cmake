message(STATUS "Resolving dependencies. This may take a while")
list(APPEND CMAKE_MODULE_PATH "${MODULE_PATH}")
include(Win32ResolveDependencies)

resolve_dependencies("${CMAKE_BINARY_DIR}/libs/hooks/harpoonHelper/src/WorkraveHelper.exe" dependencies resolved_dependencies "${SYS_ROOT}/bin;${CMAKE_INSTALL_PREFIX}/bin32")

foreach(dependency ${resolved_dependencies})
  get_filename_component(file ${dependency} NAME)
  file (INSTALL ${dependency} DESTINATION "${CMAKE_INSTALL_PREFIX}/${BINDIR32}")
endforeach()
