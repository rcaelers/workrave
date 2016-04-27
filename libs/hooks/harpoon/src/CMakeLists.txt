if (WIN32 AND (NOT PREBUILT_PATH))
  set(SOURCES
    harpoon.c
    harpoon.def
  )

  include_directories(
    ../include
  )

  add_definitions(
    -DHARPOON_EXPORTS
    -DWIN32
    -DWINVER=0x400
    -D_USRDLL
    -D_WIN32_WINNT=0x500
    -D_WINDOWS
  )

  if (PLATFORM_ARCH EQUAL 32)
    set(HARPOON_TARGET "harpoon")
  else()
    set(HARPOON_TARGET "harpoon64")
  endif()

  add_library(${HARPOON_TARGET} SHARED ${SOURCES})
  set_target_properties(${HARPOON_TARGET} PROPERTIES PREFIX "")
  target_link_libraries(${HARPOON_TARGET} kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32)
  install(TARGETS ${HARPOON_TARGET} RUNTIME DESTINATION lib)
endif()
