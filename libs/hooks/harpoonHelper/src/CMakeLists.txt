if (WIN32 AND (PLATFORM_ARCH EQUAL 32))
  set(SOURCES
    Config.cpp
    Debug.cpp
    HarpoonHelper.cpp
    main.cpp
  )

  add_executable(WorkraveHelper ${SOURCES})
  target_link_libraries(WorkraveHelper harpoon)
  target_include_directories(WorkraveHelper PRIVATE "../../harpoon/include"  "../include")
  target_compile_definitions(WorkraveHelper PRIVATE PLATFORM_OS_WINDOWS_NATIVE)
  install(TARGETS WorkraveHelper RUNTIME DESTINATION ${BINDIR32})

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(WorkraveHelper PROPERTIES COMPILE_FLAGS "-Wl,--subsystem,windows")
    set_target_properties(WorkraveHelper PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(WorkraveHelper PROPERTIES COMPILE_FLAGS "-mwindows")
    set_target_properties(WorkraveHelper PROPERTIES LINK_FLAGS "-mwindows")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(WorkraveHelper PROPERTIES COMPILE_FLAGS "/SUBSYSTEM:WINDOWS")
    set_target_properties(WorkraveHelper PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  endif()

  install(CODE "set(SYS_ROOT ${SYS_ROOT})")
  install(CODE "set(CMAKE_OBJDUMP ${CMAKE_OBJDUMP})")
  install(CODE "set(MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)")
  install(CODE "set(DIST_PATH ${CMAKE_CURRENT_BINARY_DIR})")
  install(CODE "set(BINDIR \"${BINDIR}\")")
  install(CODE "set(BINDIR32 \"${BINDIR32}\")")
  install(CODE "set(BINARY_DIR \"${CMAKE_BINARY_DIR}\")")

  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Install.cmake")
endif()
