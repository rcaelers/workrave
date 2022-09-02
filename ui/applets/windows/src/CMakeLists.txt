if (WIN32)
  if (PLATFORM_ARCH EQUAL 32)
    set(APPLET_TARGET "workrave-applet")
    set(APPLET_INSTALL_DIR ${BINDIR32})
  else()
    set(APPLET_TARGET "workrave-applet64")
    set(APPLET_INSTALL_DIR ${BINDIR})
  endif()

  add_library(${APPLET_TARGET} SHARED
              Applet.cpp
              ClsFact.cpp
              Debug.cpp
              DeskBand.cpp
              Icon.cpp
              PaintHelper.cpp
              TimeBar.cpp
              TimerBox.cpp
              Util.cpp
              resource.rc
              Applet.def
  )

  target_include_directories(
    ${APPLET_TARGET}
    PRIVATE
    ../include
    ${CMAKE_SOURCE_DIR}/libs/utils/include
    ${CMAKE_SOURCE_DIR}/libs/core/include
    ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include
  )

  target_compile_definitions(
    ${APPLET_TARGET}
    PRIVATE
    -DWIN32
    -DWINVER=0xA00
    -D_USRDLL
    -D_WIN32_WINNT=0x0A00
    -D_WINDOWS
  )

  if (NOT MSVC)
    target_compile_options(${APPLET_TARGET} PRIVATE -Wno-missing-exception-spec)
  endif()

  if (WITH_TRACING)
    target_compile_definitions(${APPLET_TARGET} PRIVATE -DTRACING)
  endif()

  set_target_properties(${APPLET_TARGET} PROPERTIES PREFIX "")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
     set_target_properties(${APPLET_TARGET} PROPERTIES LINK_FLAGS "-Wl,--allow-multiple-definition" )
  endif()
  target_link_libraries(${APPLET_TARGET} Boost::serialization shlwapi shcore uuid)

  install(TARGETS ${APPLET_TARGET}
          RUNTIME DESTINATION ${APPLET_INSTALL_DIR}
  )
endif()
