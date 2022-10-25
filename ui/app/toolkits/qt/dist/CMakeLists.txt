execute_process(COMMAND date "+%Y%m%d" OUTPUT_VARIABLE CPACK_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# Get the build revision, note that this can get out-of-sync, so for packaging run cmake first.
set (MY_WC_HASH "unknown")
if (EXISTS ${CMAKE_SOURCE_DIR}/.git/)
  find_package(Git)
  if (GIT_FOUND)
    message(STATUS "Found Git: ${GIT_EXECUTABLE}")
    execute_process(COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE MY_WC_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
  endif()
endif()
set (BUILD_REV ${MY_WC_HASH})
unset (MY_WC_HASH)

set(CPACK_PACKAGE_NAME                  "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR                "Rob Caelers, Raymond Penners, Ray Satiro")
set(CPACK_PACKAGE_CONTACT               "https://github.com/rcaelers/workrave/discussions")
set(CPACK_PACKAGE_EXECUTABLES           "${PROJECT_NAME};${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION               "${WORKRAVE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "Repetitive Strain Injury prevention tool")
set(CPACK_PACKAGE_DESCRIPTION_FILE      "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_FILE_NAME             "${PROJECT_NAME_LOWER}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}-git${CPACK_DATE}.${BUILD_REV}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY     "${PROJECT_NAME}")
set(CPACK_PACKAGE_ICON                  "${CMAKE_SOURCE_DIR}/ui/data/images/macos/workrave.icns")
set(CPACK_PACKAGE_DIRECTORY             "${CMAKE_BINARY_DIR}")
set(CPACK_STRIP_FILES                   TRUE)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if (WIN32)
  set (CPACK_RESOURCE_FILE_LICENSE      "${CMAKE_BINARY_DIR}/COPYING.txt")
else()
  set (CPACK_RESOURCE_FILE_LICENSE      "${CMAKE_SOURCE_DIR}/COPYING")
endif()

set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

add_custom_target(bundle
                  COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
                  COMMENT "Running CPACK. Please wait..."
                  DEPENDS ${PROJECT_NAME})
set(CPACK_GENERATOR)

if(PLATFORM_OS_MACOS)
  add_subdirectory(macos)
elseif(PLATFORM_OS_WINDOWS)
  add_subdirectory(windows)
endif()

include(CPack)
