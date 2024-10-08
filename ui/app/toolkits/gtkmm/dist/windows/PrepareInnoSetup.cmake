string(REPLACE "/" "\\" INSTALL_WIN_PATH "${INSTALL_PATH}")

set(LIBS_ISS "${DIST_PATH}/libraries.iss")

file(WRITE ${LIBS_ISS} "; 64 bit\n")
file(GLOB FILES64BIT "${CMAKE_INSTALL_PREFIX}/${BINDIR}/*.dll")
foreach(file ${FILES64BIT})
  get_filename_component(file ${file} NAME)
  set(flags "ignoreversion restartreplace uninsrestartdelete")
  if (${file} MATCHES "applet.*\\.dll$")
    set(flags "${flags} regserver; Check: IsAdmin")
  endif()
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALL_WIN_PATH}\\${BINDIR}\\${file}\"; DestDir: \"{app}\\${BINDIR}\"; Flags: ${flags};\n")
endforeach()

file(APPEND ${LIBS_ISS} "\n; 32 bit\n")
file(GLOB FILES32BIT "${CMAKE_INSTALL_PREFIX}/${BINDIR32}/*.*")
foreach(file ${FILES32BIT})
  get_filename_component(file ${file} NAME)
  set(flags "ignoreversion restartreplace uninsrestartdelete 32bit")
  if (${file} MATCHES "applet.*\\.dll$")
    set(flags "${flags} regserver; Check: IsAdmin")
  endif()
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALL_WIN_PATH}\\${BINDIR32}\\${file}\"; DestDir: \"{app}\\${BINDIR32}\"; Flags: ${flags};\n")
endforeach()

if (HAVE_CRASHPAD)
  file(APPEND ${LIBS_ISS} "\n; Crashpad\n")
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALL_WIN_PATH}\\${BINDIR}\\WorkraveCrashHandler.exe\"; DestDir: \"{app}\\${BINDIR}\"; DestName: \"WorkraveCrashHandler.exe\"; Flags: ignoreversion;\n")
endif()

if (HAVE_SBOM)
  file(APPEND ${LIBS_ISS} "\n; SBOM\n")
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALL_WIN_PATH}\\sbom.csv\"; DestDir: \"{app}\"; Flags: ignoreversion;\n")
endif()
