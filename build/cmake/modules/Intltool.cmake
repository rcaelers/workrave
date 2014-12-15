if (WIN32 AND NOT CMAKE_CROSSCOMPILING)
  find_program(INTLTOOL_UPDATE_EXEC intltool-update)
  find_program(INTLTOOL_MERGE_EXEC intltool-merge)
  find_program(INTLTOOL_EXTRACT_EXEC intltool-extract)
  find_package(Perl)

  if (PERL_FOUND)
    if (INTLTOOL_MERGE_EXEC)
      set(INTLTOOL_MERGE_EXECUTABLE ${PERL_EXECUTABLE} ${INTLTOOL_MERGE_EXEC})
    endif()
    if (INTLTOOL_UPDATE_EXEC)
      set(INTLTOOL_UPDATE_EXECUTABLE ${PERL_EXECUTABLE} ${INTLTOOL_UPDATE_EXEC})
    endif()
    if (INTLTOOL_EXTRACT_EXEC)
      set(INTLTOOL_EXTRACT_EXECUTABLE ${PERL_EXECUTABLE} ${INTLTOOL_EXTRACT_EXEC})
    endif()
  endif()
else()
  find_program(INTLTOOL_UPDATE_EXECUTABLE intltool-update)
  find_program(INTLTOOL_MERGE_EXECUTABLE intltool-merge)
  find_program(INTLTOOL_EXTRACT_EXECUTABLE intltool-extract)
endif()

if (INTLTOOL_MERGE_EXECUTABLE)
  set(INTLTOOL_MERGE_FOUND ON)
  
  macro(INTLTOOL_MERGE_XML IN OUT)
    add_custom_command(
      OUTPUT ${OUT}
      COMMAND ${INTLTOOL_MERGE_EXECUTABLE} -x -u -c ${CMAKE_CURRENT_BINARY_DIR}/intltool-merge-cache ${CMAKE_SOURCE_DIR}/po ${IN} ${OUT}
      DEPENDS ${IN}
      )
  endmacro()

  macro(INTLTOOL_MERGE_DESKTOP IN OUT)
    add_custom_command(
      OUTPUT ${OUT}
      COMMAND ${INTLTOOL_MERGE_EXECUTABLE} -d -u -c ${CMAKE_CURRENT_BINARY_DIR}/intltool-merge-cache ${CMAKE_SOURCE_DIR}/po ${IN} ${OUT}
      DEPENDS ${IN}
      )
  endmacro()

  macro(INTLTOOL_NOMERGE_XML IN OUT)
    add_custom_command(
      OUTPUT ${OUT}
      COMMAND ${INTLTOOL_MERGE_EXECUTABLE} -x -u --no-translations -c ${CMAKE_CURRENT_BINARY_DIR}/intltool-merge-cache ${IN} ${OUT}
      DEPENDS ${IN}
      )
  endmacro()
endif()
