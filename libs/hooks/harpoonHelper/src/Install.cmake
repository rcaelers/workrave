message(STATUS "Resolving dependencies (Harpoon). This may take a while")
list(APPEND CMAKE_MODULE_PATH "${MODULE_PATH}")
include(Win32ResolveDependencies)

set(RUNTIME32_INSTALLERS_LOG "${BINARY_DIR}/runtime32_installers.txt")

resolve_dependencies("${CMAKE_INSTALL_PREFIX}/${BINDIR32}/WorkraveHelper.exe" dependencies resolved_dependencies "${SYS_ROOT}/bin;${CMAKE_INSTALL_PREFIX}/bin32")

file(WRITE "${RUNTIME32_INSTALLERS_LOG}" "")
foreach(dependency ${resolved_dependencies})
  get_filename_component(file ${dependency} NAME)
  file (INSTALL ${dependency} DESTINATION "${CMAKE_INSTALL_PREFIX}/${BINDIR32}")
  file(APPEND "${RUNTIME32_INSTALLERS_LOG}" "${dependency},${BINDIR32}\n")
endforeach()
