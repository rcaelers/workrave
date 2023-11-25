if (HAVE_CRASHPAD AND HAVE_TESTS)
  add_executable(crash crash.cc)

  target_link_libraries(crash workrave-libs-crash)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(crash PRIVATE -gcodeview)
    target_link_options(crash PRIVATE -Wl,-pdb=)
  endif()

  if (MINGW)
    if (DUMP_SYMS)
      add_custom_command(TARGET crash POST_BUILD
                        COMMAND ${DUMP_SYMS} $<TARGET_FILE:crash> -o ${CMAKE_BINARY_DIR}/crash.sym)
      add_dependencies(crash dump_syms)
    endif()

  endif()

  install(TARGETS crash RUNTIME DESTINATION ${BINDIR} BUNDLE DESTINATION .)
endif()
