if (PLATFORM_OS_WINDOWS)
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scalable/
  DESTINATION ${IMAGESDIR}
  FILES_MATCHING
  PATTERN "*.svg"
  )
endif()
