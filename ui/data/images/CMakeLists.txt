FILE(GLOB images
  ${CMAKE_CURRENT_SOURCE_DIR}/*.xpm
  ${CMAKE_CURRENT_SOURCE_DIR}/*.png
  ${CMAKE_CURRENT_SOURCE_DIR}/*.svg
  ${CMAKE_CURRENT_SOURCE_DIR}/*.ico)

install(FILES ${images} DESTINATION ${IMAGESDIR})

add_subdirectory(adwaita)
add_subdirectory(workrave)
