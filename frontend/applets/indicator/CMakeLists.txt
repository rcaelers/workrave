set(SRC
  src/indicator-workrave.c
 )

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/applets/common/include
  ${CMAKE_SOURCE_DIR}/frontend/applets/indicator/include
  )

add_definitions(-DWORKRAVE_PKGDATADIR="${PKGDATADIR}")

include_directories(
  ${GTK_INCLUDE_DIRS}
  ${INDICATOR_INCLUDE_DIRS}
  )

add_library(workrave-frontend-applet-indicator STATIC ${SRC})

link_directories(${INDICATOR_LIBPATH})
target_link_libraries(workrave-frontend-applet-indicator ${INDICATOR_LIBRARIES})

