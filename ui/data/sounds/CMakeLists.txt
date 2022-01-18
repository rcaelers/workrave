set (SOUND_THEMES classic subtle bells)

foreach(soundtheme ${SOUND_THEMES})
  file(GLOB SND_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/${soundtheme}/*.wav
                       ${CMAKE_CURRENT_SOURCE_DIR}/${soundtheme}/soundtheme)

  install(FILES ${SND_FILES} DESTINATION ${SOUNDSDIR}/${soundtheme}
  )
endforeach()
