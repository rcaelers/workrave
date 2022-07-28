install(CODE "set(SYS_ROOT ${SYS_ROOT})")
install(CODE "set(CMAKE_OBJDUMP ${CMAKE_OBJDUMP})")
install(CODE "set(MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)")
install(CODE "set(DIST_PATH ${CMAKE_CURRENT_BINARY_DIR})")

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Install.cmake")
