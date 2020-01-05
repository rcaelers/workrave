set(SRC
  Configurator.cc
  ConfiguratorFactory.cc
  IniConfigurator.cc
  XmlConfigurator.cc
  SettingCache.cc
  )

if (PLATFORM_OS_WINDOWS)
  set(SRC ${SRC} win32/W32Configurator.cc)
  include_directories(${CMAKE_SOURCE_DIR}/libs/config/src/win32)
elseif (PLATFORM_OS_OSX)
  set(SRC ${SRC} osx/OSXConfigurator.cc)
  include_directories(${CMAKE_SOURCE_DIR}/libs/config/src/osx)
  set_source_files_properties(osx/OSXConfigurator.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")  
else()

if (HAVE_QT5)
  set(SRC ${SRC} QtSettingsConfigurator.cc)
endif()

if (HAVE_GSETTINGS) 
  set(SRC ${SRC}   GSettingsConfigurator.cc)
endif()

if (WITH_GCONF)
  set(SRC ${SRC} unix/GConfConfigurator.cc)
  include_directories(${GCONF_INCLUDE_DIRS})
  include_directories(${CMAKE_SOURCE_DIR}/libs/config/src/unix)
endif()

endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/config/src
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-config STATIC ${SRC})
add_cppcheck(workrave-libs-config)

if (HAVE_GCONF)
  link_directories(${GCONF_LIBPATH})
  target_link_libraries(workrave-libs-config ${GCONF_LIBRARIES})
endif()

if (PLATFORM_OS_OSX)
  target_link_libraries(workrave-libs-config ${COCOA_LIBRARY})
endif()
