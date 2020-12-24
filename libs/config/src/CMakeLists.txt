add_library(workrave-libs-config STATIC 
  Configurator.cc
  ConfiguratorFactory.cc
  IniConfigurator.cc
  XmlConfigurator.cc
  SettingCache.cc)

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-config PRIVATE win32/W32Configurator.cc)
  target_include_directories(workrave-libs-config PRIVATE ${CMAKE_SOURCE_DIR}/libs/config/src/win32)
elseif (PLATFORM_OS_MACOS)
  target_sources(workrave-libs-config PRIVATE macos/MacOSConfigurator.cc)
  target_include_directories(workrave-libs-config PRIVATE ${CMAKE_SOURCE_DIR}/libs/config/src/macos)
  target_link_libraries(workrave-libs-config ${COCOA_LIBRARY})
  set_source_files_properties(macos/MacOSConfigurator.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

if (HAVE_QT5)
  target_sources(workrave-libs-config PRIVATE QtSettingsConfigurator.cc)
  target_include_directories(workrave-libs-config PRIVATE ${Qt5Core_INCLUDE_DIRS})
endif()

if (HAVE_GSETTINGS)
  target_sources(workrave-libs-config PRIVATE GSettingsConfigurator.cc)
endif()

target_link_libraries(workrave-libs-config workrave-libs-utils)

target_include_directories(workrave-libs-config
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/config/src
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/config/include)

