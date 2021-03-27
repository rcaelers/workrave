if (WIN32 AND (NOT HAVE_PREBUILT_VISUAL_STUDIO) AND (PLATFORM_ARCH EQUAL 32))
  set(SOURCES
    Config.cpp
    Debug.cpp
    HarpoonHelper.cpp
    main.cpp
  )

  include_directories(
    ../../harpoon/include
    ../include
  )

  add_definitions(
    -DWIN32
    -DWINVER=0x400
    -D_WIN32_WINNT=0x500
    -D_WINDOWS
    -DPLATFORM_OS_WINDOWS_NATIVE
  )

  add_executable(WorkraveHelper ${SOURCES})
  target_link_libraries(WorkraveHelper harpoon)
  install(TARGETS WorkraveHelper RUNTIME DESTINATION lib32)

  set_target_properties(WorkraveHelper PROPERTIES COMPILE_FLAGS "-mwindows")
  set_target_properties(WorkraveHelper PROPERTIES LINK_FLAGS "-mwindows")
endif()
