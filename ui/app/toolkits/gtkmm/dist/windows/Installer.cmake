if (POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif()
if (POLICY CMP0012)
  cmake_policy(SET CMP0012 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${MODULE_PATH}")
string(REPLACE "/" "\\" INSTALLDIR ${CMAKE_INSTALL_PREFIX})

message(STATUS "Resolving dependencies. This may take a while")
include(Win32ResolveDependencies)

resolve_dependencies("${CMAKE_INSTALL_PREFIX}/lib/workrave.exe" dependencies resolved_dependencies ${SYS_ROOT}/bin)
resolve_dependencies("${CMAKE_INSTALL_PREFIX}/lib/WorkraveCrashHandler.exe" dependencies resolved_dependencies ${SYS_ROOT}/bin)

file(GLOB PLUGINS "${CMAKE_INSTALL_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll")
foreach(plugin ${PLUGINS})
  resolve_dependencies("${plugin}" dependencies resolved_dependencies ${SYS_ROOT}/bin)
endforeach()

set(LIBS_ISS "${DIST_PATH}/libraries.iss")
file(WRITE ${LIBS_ISS} "; Resolved from Workrave.exe\n")
foreach(dependency ${resolved_dependencies})
  get_filename_component(file ${dependency} NAME)
  file (INSTALL ${dependency} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALLDIR}\\lib\\${file}\"; DestDir: \"{app}\\lib\"; Flags: ignoreversion;\n")
endforeach()

file(INSTALL ${SYS_ROOT}/bin/gdbus.exe DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

file(APPEND ${LIBS_ISS} "; 32 bit\n")
file(GLOB FILES32BIT "${CMAKE_INSTALL_PREFIX}/lib32/*.*")
foreach(file ${FILES32BIT})
  get_filename_component(file ${file} NAME)
  set(flags "ignoreversion 32bit")
  if (${file} MATCHES "applet.*\\.dll$")
    set(flags "${flags} restartreplace uninsrestartdelete regserver; Components: applet")
  endif()
  if (${file} MATCHES "^harpoon\\.dll$")
    set(flags "${flags} restartreplace uninsrestartdelete regserver")
  endif()
  if (${file} MATCHES "^WorkraveHelper.exe$")
    set(flags "${flags} restartreplace uninsrestartdelete regserver")
  endif()
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALLDIR}\\lib32\\${file}\"; DestDir: \"{app}\\lib32\"; Flags: ${flags};\n")
endforeach()

if (WINE)
  execute_process(
    COMMAND "${WINE}" "${ISCC}" "/o${CMAKE_INSTALL_PREFIX}" setup.iss
    OUTPUT_VARIABLE out
    RESULT_VARIABLE ret
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY "${DIST_PATH}"
    )
else()
  message("Running ${ISCC}")
  execute_process(
    COMMAND "${ISCC}" "/o${CMAKE_INSTALL_PREFIX}" setup.iss
    OUTPUT_VARIABLE out
    RESULT_VARIABLE ret
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY "${DIST_PATH}"
    )
endif()

message("Setup: ${out}")
if (NOT "${ret}" STREQUAL "0")
  message(FATAL_ERROR "ISCC failed (${ret})")
endif()

