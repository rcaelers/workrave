if (WIN32) 
  if (NOT HAVE_PREBUILT_VISUAL_STUDIO)
    if (PLATFORM_ARCH EQUAL 32)
      set(HARPOON_TARGET "harpoon")
      set(HARPOON_INSTALL_DIR "lib32")
    else()
      set(HARPOON_TARGET "harpoon64")
      set(HARPOON_INSTALL_DIR "lib")
    endif()

    set(SOURCES
      harpoon.c
      harpoon.def
    )
    add_library(${HARPOON_TARGET} SHARED ${SOURCES})  

    target_compile_definitions(${HARPOON_TARGET} PRIVATE 
      -DHARPOON_EXPORTS
      -DWIN32
      -DWINVER=0x400
      -D_USRDLL
      -D_WIN32_WINNT=0x500
      -D_WINDOWS
    )
    set_target_properties(${HARPOON_TARGET} PROPERTIES PREFIX "")
    target_link_libraries(${HARPOON_TARGET} kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32)
    install(TARGETS ${HARPOON_TARGET} RUNTIME DESTINATION ${HARPOON_INSTALL_DIR})
  endif()
  target_include_directories(${HARPOON_TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()