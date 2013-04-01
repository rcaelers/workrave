intltool_merge_xml(
  ${CMAKE_CURRENT_SOURCE_DIR}/common/share/exercises.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml
  )

file(GLOB EXERCISE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/common/share/*.png)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml DESTINATION share/workrave/exercises)
install(FILES ${EXERCISE_FILES} DESTINATION share/workrave/exercises)

add_custom_target(generate_exercises_xml ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml)