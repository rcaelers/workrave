if (WITH_INDICATOR)

  set(SRC
    src/indicator-workrave.c
    )

  include_directories(${GTK_INCLUDE_DIRS})

  include_directories(
    ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
    )

  include_directories(
    ${GTK_INCLUDE_DIRS}
    ${INDICATOR_INCLUDE_DIRS}
    )

  add_library(workrave-applet-indicator SHARED ${SRC})
  
  set_target_properties(workrave-applet-indicator PROPERTIES OUTPUT_NAME workrave)

  link_directories(${INDICATOR_LIBPATH})
  target_link_libraries(workrave-applet-indicator ${INDICATOR_LIBRARIES} workrave-private-1.0)

  install(TARGETS workrave-applet-indicator RUNTIME DESTINATION ${INDICATORDIR} LIBRARY DESTINATION ${INDICATORDIR})
  #set_target_properties(workrave-private PROPERTIES VERSION 1.0 SOVERSION 1.0)

endif()
