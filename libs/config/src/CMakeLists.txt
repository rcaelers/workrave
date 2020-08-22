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
elseif (PLATFORM_OS_MACOS)
  set(SRC ${SRC} macos/MacOSConfigurator.cc)
  include_directories(${CMAKE_SOURCE_DIR}/libs/config/src/macos)
  set_source_files_properties(macos/MacOSConfigurator.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
else()

if (HAVE_QT5)
  set(SRC ${SRC} QtSettingsConfigurator.cc)
endif()

if (HAVE_GSETTINGS)
  set(SRC ${SRC}   GSettingsConfigurator.cc)
endif()

endif()

add_library(workrave-libs-config STATIC ${SRC})
add_cppcheck(workrave-libs-config)

target_link_libraries(workrave-libs-config workrave-libs-utils)

if (HAVE_QT5)
  target_include_directories(workrave-libs-config PRIVATE ${Qt5Core_INCLUDE_DIRS})
endif()

if (PLATFORM_OS_MACOS)
  target_link_libraries(workrave-libs-config ${COCOA_LIBRARY})
endif()

target_include_directories(workrave-libs-config
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/config/src
  ${CMAKE_SOURCE_DIR}/libs/config/include/config
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/config/include
  )

