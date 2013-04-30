set(SRC
  src/Configurator.cc
  src/ConfiguratorFactory.cc
  src/GSettingsConfigurator.cc
  src/GlibIniConfigurator.cc
  )

if (HAVE_QT5)
  set(SRC ${SRC} src/QtSettingsConfigurator.cc)
endif()

if (HAVE_GDOME)
  set(SRC ${SRC} src/XMLConfigurator.cc)
  include_directories(${GDOME_INCLUDE_DIRS})
endif()

if (WITH_GCONF)
  set(SRC ${SRC} src/unix/GConfConfigurator.cc)
  include_directories(${GCONF_INCLUDE_DIRS})
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} src/win32/W32Configurator.cc)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC} src/osx/OSXConfigurator.ccx)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/config/src
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-config STATIC ${SRC})

if (HAVE_GDOME)
  link_directories(${GDOME_LIBPATH})
  target_link_libraries(workrave-libs-config ${GDOME_LIBRARIES})
endif()

if (HAVE_GCONF)
  link_directories(${GCONF_LIBPATH})
  target_link_libraries(workrave-libs-config ${GCONF_LIBRARIES})
endif()
