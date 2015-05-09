if (WITH_INDICATOR AND HAVE_GTK3)
  
  set(SRC
    src/indicator-workrave.c
    )
  
  include_directories(${GTK_INCLUDE_DIRS})
  
  include_directories(
    ${CMAKE_SOURCE_DIR}/frontend/applets/common/include
    ${CMAKE_SOURCE_DIR}/frontend/applets/indicator/include
    )
  
  include_directories(
    ${GTK_INCLUDE_DIRS}
    ${INDICATOR_INCLUDE_DIRS}
    )
  
  add_library(workrave-frontend-applet-indicator SHARED ${SRC})
  add_cppcheck(workrave-frontend-applet-indicator)
  
  set_target_properties(workrave-frontend-applet-indicator PROPERTIES OUTPUT_NAME workrave)
  
  link_directories(${INDICATOR_LIBPATH})
  target_link_libraries(workrave-frontend-applet-indicator ${INDICATOR_LIBRARIES})
  
  install(TARGETS workrave-frontend-applet-indicator RUNTIME DESTINATION ${INDICATORDIR} LIBRARY DESTINATION ${INDICATORDIR})
  #set_target_properties(workrave-private PROPERTIES VERSION 1.0 SOVERSION 1.0)
  
endif()
