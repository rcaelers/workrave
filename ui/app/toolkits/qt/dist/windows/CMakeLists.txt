include(InstallRequiredSystemLibraries)

set(plugin_dest_dir "lib/plugins")
set(qtconf_dest_dir "bin")

set(IMAGE_PLUGINS QJpegPlugin QGifPlugin)
set(PLATFORM_PLUGINS QMinimalIntegrationPlugin QWindowsIntegrationPlugin)

# foreach(plugin ${PLATFORM_PLUGINS})
#   get_target_property(_loc Qt6::${plugin} LOCATION_${CMAKE_BUILD_TYPE})
#   if (_loc)
#     install(FILES ${_loc} DESTINATION ${plugin_dest_dir}/platforms)
#   endif()
# endforeach()

# foreach(plugin ${IMAGE_PLUGINS})
#   get_target_property(_loc Qt6::${plugin} LOCATION_${CMAKE_BUILD_TYPE})
#   if (_loc)
#     install(FILES ${_loc} DESTINATION ${plugin_dest_dir}/imageformats)
#   endif()
# endforeach()


install(FILES WizModernImage.bmp DESTINATION dist)
install(FILES WizModernSmall.bmp DESTINATION dist)

install(DIRECTORY "${QT_PLUGINS_DIR}/platforms"     DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats"  DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/iconengines"   DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
install(DIRECTORY "${QT_PLUGINS_DIR}/styles"        DESTINATION ${plugin_dest_dir} COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)

# Install a qt.conf file.
# This inserts some cmake code into the install script to write the file
install(CODE "
  file(WRITE \"${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = ../lib/plugins/\")
  " COMPONENT Runtime)

install(CODE "set(SYS_ROOT ${SYS_ROOT})")
install(CODE "set(MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)")
install(CODE "set(DIST_PATH \"${CMAKE_CURRENT_BINARY_DIR}\")")
install(CODE "set(INSTALL_PATH \"${CMAKE_INSTALL_PREFIX}\")")
install(CODE "set(DEP_DIRS \"${CMAKE_INSTALL_PREFIX}/bin;${SYS_ROOT}/bin;${spdlog_BINARY_DIR}\")")
install(CODE "set(BINDIR \"${BINDIR}\")")
install(CODE "set(BINDIR32 \"${BINDIR32}\")")

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/InstallDependencies.cmake")
install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/PrepareInnoSetup.cmake")

string(REPLACE "/" "\\" INSTALL_WIN_PATH "${CMAKE_INSTALL_PREFIX}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.iss.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.iss"
  @ONLY
)

set(INSTALLER_NAME "workrave-installer")
set(INSTALLER_TARGET "${INSTALLER_NAME}.exe")
set(ISCC_FILENAME_FLAGS "/F${INSTALLER_NAME}")

if(WINE)
  add_custom_command(OUTPUT ${INSTALLER_TARGET}
                      COMMAND "${WINE}" "${ISCC}" setup.iss "/o${CMAKE_INSTALL_PREFIX}" "/DCHANNEL=${WORKRAVE_CHANNEL}" ${ISCC_FLAGS} ${ISCC_FILENAME_FLAGS}
                      DEPENDS workrave)
else()
  add_custom_command(OUTPUT ${INSTALLER_TARGET}
                      COMMAND "${ISCC}" setup.iss "/o${CMAKE_INSTALL_PREFIX}" "/DCHANNEL=${WORKRAVE_CHANNEL}" ${ISCC_FLAGS} ${ISCC_FILENAME_FLAGS}
                      DEPENDS workrave
                      VERBATIM)
endif()

add_custom_target(installer DEPENDS ${INSTALLER_TARGET})

if (CMAKE_CROSSCOMPILING)
  ExternalProject_Add(ZapperQt32Bit
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.32
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchains/mingw32-gcc.cmake
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
  )
elseif (MSVC)
  ExternalProject_Add(ZapperQt32Bit
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.32
    CMAKE_GENERATOR "Visual Studio 17 2022"
    CMAKE_ARGS -A Win32
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
      -DVCPKG_TARGET_TRIPLET:STRING=x86-windows
  )
elseif (MINGW)
  set(CMAKE_COMMAND32 ${MSYS_CMD} -here -mingw32 -no-start -defterm -c "cmake \$*" cmake)
  ExternalProject_Add(ZapperQt32Bit
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.32
    CMAKE_COMMAND ${CMAKE_COMMAND32}
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchains/msys2.cmake
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
  )
endif()
