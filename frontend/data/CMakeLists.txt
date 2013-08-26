foreach(size 16x16 24x24 32x32 48x48 64x64 96x96 128x128 scalable)
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/images/${size}/
    DESTINATION share/icons/hicolor/${size}/apps/
    FILES_MATCHING
    PATTERN "*.png"
    PATTERN "*.svg"
    )
endforeach()


FILE(GLOB images
  ${CMAKE_CURRENT_SOURCE_DIR}/images/*.png
  ${CMAKE_CURRENT_SOURCE_DIR}/images/*.svg
  ${CMAKE_CURRENT_SOURCE_DIR}/images/*.ico)
  
install(FILES ${images} DESTINATION share/workrave/images)

# TODO: install win32/osx specific files.

set (SOUND_THEMES default subtle)

foreach(soundtheme ${SOUND_THEMES})
  file(GLOB SND_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/sounds/${soundtheme}/*.wav
	  	       ${CMAKE_CURRENT_SOURCE_DIR}/sounds/${soundtheme}/soundtheme)

  install(FILES ${SND_FILES} DESTINATION share/sounds/workrave/${soundtheme}
  )
endforeach()

intltool_merge_xml(
  ${CMAKE_CURRENT_SOURCE_DIR}/exercises/exercises.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml
  )

file(GLOB EXERCISE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/exercises/*.png)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml DESTINATION share/workrave/exercises)
install(FILES ${EXERCISE_FILES} DESTINATION share/workrave/exercises)

add_custom_target(generate_exercises_xml ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml)