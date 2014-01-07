intltool_merge_xml(
  ${CMAKE_CURRENT_SOURCE_DIR}/exercises.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml
  )

file(GLOB EXERCISE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.png)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml DESTINATION ${EXERCISESDIR})
install(FILES ${EXERCISE_FILES} DESTINATION ${EXERCISESDIR})

add_custom_target(generate_exercises_xml ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml)