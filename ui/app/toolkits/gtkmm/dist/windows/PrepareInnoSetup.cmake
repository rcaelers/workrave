string(REPLACE "/" "\\" INSTALL_WIN_PATH "${INSTALL_PATH}")

set(LIBS_ISS "${DIST_PATH}/libraries.iss")

file(WRITE ${LIBS_ISS} "; 64 bit\n")
file(GLOB FILES64BIT "${CMAKE_INSTALL_PREFIX}/lib/*.dll")
foreach(file ${FILES64BIT})
  get_filename_component(file ${file} NAME)
  set(flags "ignoreversion restartreplace uninsrestartdelete")
  if (${file} MATCHES "applet.*\\.dll$")
    set(flags "${flags} regserver; Components: applet; Check: IsAdmin")
  endif()
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALL_WIN_PATH}\\lib\\${file}\"; DestDir: \"{app}\\lib\"; Flags: ${flags};\n")
endforeach()

file(APPEND ${LIBS_ISS} "\n; 32 bit\n")
file(GLOB FILES32BIT "${CMAKE_INSTALL_PREFIX}/lib32/*.*")
foreach(file ${FILES32BIT})
  get_filename_component(file ${file} NAME)
  set(flags "ignoreversion restartreplace uninsrestartdelete 32bit")
  if (${file} MATCHES "applet.*\\.dll$")
    set(flags "${flags} regserver; Components: applet; Check: IsAdmin")
  endif()
  file(APPEND ${LIBS_ISS} "Source: \"${INSTALL_WIN_PATH}\\lib32\\${file}\"; DestDir: \"{app}\\lib32\"; Flags: ${flags};\n")
endforeach()

if (HAVE_CRASHPAD)
  file(APPEND ${LIBS_ISS} "Source: \"@INSTALL_WIN_PATH@\\lib\\WorkraveCrashHandler.exe\"; DestDir: \"{app}\\lib\"; DestName: \"WorkraveCrashHandler.exe\"; Flags: ignoreversion;\n")
endif()
