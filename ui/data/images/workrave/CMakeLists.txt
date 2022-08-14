if (HAVE_APP_GTK)
  foreach(size 16x16 24x24 32x32 48x48 64x64 96x96 128x128 scalable)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${size}/
      DESTINATION ${DATADIR}/icons/hicolor/${size}/apps
      FILES_MATCHING
      PATTERN "*.png"
      PATTERN "*.svg"
      )
  endforeach()
endif()
