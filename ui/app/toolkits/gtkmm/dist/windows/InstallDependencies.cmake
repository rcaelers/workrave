if (POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif()
if (POLICY CMP0012)
  cmake_policy(SET CMP0012 NEW)
endif()

string(REPLACE ";" "\ " MODULE_PATH "${MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${MODULE_PATH}")

string(REPLACE "/" "\\" INSTALL_WIN_PATH "${INSTALL_PATH}")

message(STATUS "Resolving dependencies. This may take a while")

if (NOT MSVC)
  include(Win32ResolveDependencies)

  resolve_dependencies("${INSTALL_PATH}/${BINDIR}/workrave.exe" dependencies resolved_dependencies "${DEP_DIRS}")

  if (HAVE_CRASHPAD)
    resolve_dependencies("${INSTALL_PATH}/${BINDIR}/WorkraveCrashHandler.exe" dependencies resolved_dependencies "${DEP_DIRS}")
  endif()
  if (HAVE_DBUS)
    resolve_dependencies("${INSTALL_PATH}/${BINDIR}/gdbus.exe" dependencies resolved_dependencies "${DEP_DIRS}")
  endif()

  file(GLOB PLUGINS "${INSTALL_PATH}/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll")
  foreach(plugin ${PLUGINS})
    resolve_dependencies("${plugin}" dependencies resolved_dependencies ${DEP_DIRS})
  endforeach()

  foreach(dependency ${resolved_dependencies})
    file(INSTALL ${dependency} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
  endforeach()

else()
  file(GLOB PLUGINS "${INSTALL_PATH}/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll")

  set(EXECUTABLES "${INSTALL_PATH}/${BINDIR}/workrave.exe")
  if (HAVE_CRASHPAD)
    list(APPEND EXECUTABLES "${INSTALL_PATH}/${BINDIR}/WorkraveCrashHandler.exe")
  endif()
  if (HAVE_DBUS)
    list(APPEND EXECUTABLES "${INSTALL_PATH}/${BINDIR}/gdbus.exe")
  endif()

  file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES
      ${EXECUTABLES}
    LIBRARIES
      ${PLUGINS}
    RESOLVED_DEPENDENCIES_VAR resolved
    UNRESOLVED_DEPENDENCIES_VAR unresolved
    PRE_EXCLUDE_REGEXES
      "^api-ms-win.*"
      "^ext-ms-.*"
    POST_EXCLUDE_REGEXES
      ".*system32/.*\\.dll"
    DIRECTORIES
      "${DEP_DIRS}"
  )
  file(INSTALL ${resolved} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()


