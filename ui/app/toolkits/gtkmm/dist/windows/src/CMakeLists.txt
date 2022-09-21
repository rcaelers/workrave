set(SRC zapper.c)

add_library(libzapper-0 SHARED ${SRC})
if (MINGW)
   set_target_properties(libzapper-0 PROPERTIES LINK_FLAGS "-static-libgcc")
endif()
if (MSVC)
  set_target_properties(libzapper-0 PROPERTIES COMPILE_FLAGS "-DWIN32_LEAN_AND_MEAN")
endif()

install (TARGETS libzapper-0 RUNTIME DESTINATION dist)
