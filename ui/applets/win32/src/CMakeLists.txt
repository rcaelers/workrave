if (WIN32 AND (NOT HAVE_PREBUILT_VISUAL_STUDIO))
  set(SOURCES Applet.cpp
              ClsFact.cpp
              Debug.cpp
              DeskBand.cpp
              Icon.cpp
              PaintHelper.cpp
              TimeBar.cpp
              TimerBox.cpp
              Util.cpp
              resource.rc
  )

  include_directories(
    ../include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    ${CMAKE_SOURCE_DIR}/libs/core/include
    ${CMAKE_SOURCE_DIR}/ui/common/include
  )

  add_definitions(
    -DWIN32
    -DWINVER=0xA00
    -D_USRDLL
    -D_WIN32_WINNT=0x0A00
    -D_WINDOWS
  )

  if (PLATFORM_ARCH EQUAL 32)
    set(APPLET_TARGET "workrave-applet")
    set(APPLET_INSTALL_DIR "lib32")
  else()
    set(APPLET_TARGET "workrave-applet64")
    set(APPLET_INSTALL_DIR "lib")
  endif()

  add_library(${APPLET_TARGET} SHARED ${SOURCES} Applet.def)
  set_target_properties(${APPLET_TARGET} PROPERTIES PREFIX "")
  if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
     set_target_properties(${APPLET_TARGET} PROPERTIES LINK_FLAGS "-Wl,--allow-multiple-definition" )
  endif()
  target_link_libraries(${APPLET_TARGET} shlwapi uuid) #   kernel32 user32 gdi32 winspool comdlg32 advapi32  shell32 ole32 oleaut32  odbc32 odbccp32)

  install(TARGETS ${APPLET_TARGET}
          RUNTIME DESTINATION ${APPLET_INSTALL_DIR}
  )
endif()
