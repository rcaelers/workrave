set (SOUND_THEMES bells bells_ensemble classic CMakeLists.txt shangri_la subtle synth_harp tinkle_bell tuned_udu woodwind_ensemble)

foreach(soundtheme ${SOUND_THEMES})
  file(GLOB SND_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/${soundtheme}/*.wav
                       ${CMAKE_CURRENT_SOURCE_DIR}/${soundtheme}/soundtheme)

  install(FILES ${SND_FILES} DESTINATION ${SOUNDSDIR}/${soundtheme}
  )
endforeach()
