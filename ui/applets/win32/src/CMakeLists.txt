if (WIN32 AND (NOT PREBUILT_PATH))
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
    #${CMAKE_SOURCE_DIR}/libs/utils/include
    ${CMAKE_SOURCE_DIR}/libs/core/include
    ${CMAKE_SOURCE_DIR}/ui/common/include
  )

  add_definitions(
    -DWIN32
    -DWINVER=0x400
    -D_USRDLL
    -D_WIN32_WINNT=0x0601
    -D_WINDOWS
  )

  if (PLATFORM_ARCH EQUAL 32)
    set(APPLET_TARGET "workrave-applet")
  else()
    set(APPLET_TARGET "workrave-applet64")
  endif()

  add_library(${APPLET_TARGET} SHARED ${SOURCES})
  set_target_properties(${APPLET_TARGET} PROPERTIES PREFIX "")

  target_link_libraries(${APPLET_TARGET} shlwapi) #   kernel32 user32 gdi32 winspool comdlg32 advapi32  shell32 ole32 oleaut32 uuid odbc32 odbccp32)

  install(TARGETS ${APPLET_TARGET}
          RUNTIME DESTINATION lib
  )
endif()
