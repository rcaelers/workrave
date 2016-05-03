if (WIN32 AND (NOT HAVE_PREBUILT_VISUAL_STUDIO) AND (PLATFORM_ARCH EQUAL 64))
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
    -DPLATFORM_OS_WIN32_NATIVE
  )

  add_executable(WorkraveHelper ${SOURCES})
  target_link_libraries(WorkraveHelper harpoon64)
  install(TARGETS WorkraveHelper RUNTIME DESTINATION lib)
endif()
