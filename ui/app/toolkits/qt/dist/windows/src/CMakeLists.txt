
cmake_minimum_required(VERSION 3.30)
project(zapper)
cmake_policy(VERSION 3.30)

set(SRC zapper.c)

add_library(zapper-0 SHARED ${SRC})
if (MINGW)
   set_target_properties(zapper-0 PROPERTIES LINK_FLAGS "-static-libgcc")
endif()
if (MSVC)
  set_target_properties(zapper-0 PROPERTIES COMPILE_FLAGS "-DWIN32_LEAN_AND_MEAN")
endif()
set_target_properties(zapper-0 PROPERTIES PREFIX "lib")

install (TARGETS zapper-0 RUNTIME DESTINATION dist)
