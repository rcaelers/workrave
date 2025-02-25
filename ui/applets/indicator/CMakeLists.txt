if (WITH_INDICATOR)

  set(SRC
    src/indicator-workrave.c
    )

  add_library(workrave-applet-indicator SHARED ${SRC})

  set_target_properties(workrave-applet-indicator PROPERTIES OUTPUT_NAME workrave)

  target_link_directories(workrave-applet-indicator PRIVATE ${INDICATOR_LIBPATH} ${DBUSMENU_LIBPATH})
  target_link_libraries(workrave-applet-indicator ${INDICATOR_LIBRARIES} ${DBUSMENU_LIBRARIES} workrave-private-1.0)

  target_include_directories(workrave-applet-indicator PRIVATE
                             ${CMAKE_SOURCE_DIR}/ui/applets/indicator/include
                             ${GTK_INCLUDE_DIRS}
                             ${INDICATOR_INCLUDE_DIRS}
                             ${DBUSMENU_INCLUDE_DIRS})

  install(TARGETS workrave-applet-indicator RUNTIME DESTINATION ${INDICATORDIR} LIBRARY DESTINATION ${INDICATORDIR})
  #set_target_properties(workrave-private PROPERTIES VERSION 1.0 SOVERSION 1.0)

endif()
