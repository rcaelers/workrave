add_library(workrave-libs-config STATIC
  Configurator.cc
  ConfiguratorFactory.cc
  IniConfigurator.cc
  XmlConfigurator.cc
  SettingCache.cc)

target_code_coverage(workrave-libs-config)

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-config PRIVATE windows/W32Configurator.cc)
  target_include_directories(workrave-libs-config PRIVATE ${CMAKE_SOURCE_DIR}/libs/config/src/windows)
elseif (PLATFORM_OS_MACOS)
  target_sources(workrave-libs-config PRIVATE macos/MacOSConfigurator.cc)
  target_include_directories(workrave-libs-config PRIVATE ${CMAKE_SOURCE_DIR}/libs/config/src/macos)
  target_link_libraries(workrave-libs-config PRIVATE ${COCOA_LIBRARY})
  set_source_files_properties(macos/MacOSConfigurator.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

if (HAVE_QT)
  target_sources(workrave-libs-config PRIVATE QtSettingsConfigurator.cc)
  target_link_libraries(workrave-libs-config PUBLIC Qt${QT_VERSION_MAJOR}::Core)
endif()

if (HAVE_GSETTINGS)
  target_sources(workrave-libs-config PRIVATE GSettingsConfigurator.cc)
  target_include_directories(workrave-libs-config PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

target_link_libraries(workrave-libs-config PRIVATE workrave-libs-utils Boost::boost)

target_include_directories(workrave-libs-config
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/config/src
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${Boost_INCLUDE_DIR})

