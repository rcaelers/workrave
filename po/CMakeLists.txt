######################################################################
## Translations
######################################################################

set(POTFILE ${CMAKE_CURRENT_BINARY_DIR}/workrave.pot)
file(GLOB POFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)

if (GETTEXT_FOUND)
  add_custom_target(update-po)
endif()

foreach (po_file ${POFILES})
  get_filename_component(language ${po_file} NAME_WE)

  if (QT_LCONVERT_EXECUTABLE)
    set(ts_file ${CMAKE_CURRENT_BINARY_DIR}/workrave_${language}.ts)
    set(qm_file ${CMAKE_CURRENT_BINARY_DIR}/workrave_${language}.qm)

    add_custom_command(
        OUTPUT ${ts_file}
        COMMAND ${QT_LCONVERT_EXECUTABLE} ARGS ${po_file} -o ${ts_file} -of ts
        DEPENDS ${po_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_command(
        OUTPUT ${qm_file}
        COMMAND ${QT_LRELEASE_EXECUTABLE} ARGS ${ts_file} -qm ${qm_file}
        DEPENDS ${ts_file})

    install(FILES ${qm_file} DESTINATION ${DATADIR}/translations/)
    set(qm_files ${qm_files} ${qm_file})
  endif()

  if (GETTEXT_FOUND)
    set(gmo_file ${CMAKE_CURRENT_BINARY_DIR}/${language}.gmo)

    # Languages to allow fuzzy translations for.
    # For example to allow german and italian fuzzy translations:  -Dlang_fuzzy:STRING=de;it
    set(OPT_USE_FUZZY "")
    if (LANG_FUZZY)
      foreach (fuzz ${LANG_FUZZY})
        if ("${fuzz}" STREQUAL "${lang}")
          set(OPT_USE_FUZZY --use-fuzzy)
          break()
        endif()
      endforeach()
    endif()

    add_custom_command(
        TARGET update-po
        POST_BUILD
        COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --no-wrap --backup=none -s ${po_file} ${POTFILE}
        DEPENDS ${POTFILE}
        VERBATIM
    )
  
    add_custom_command(
      OUTPUT ${gmo_file}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${OPT_USE_FUZZY} -c -o ${gmo_file} ${po_file}
      DEPENDS ${POTFILE} ${po_file}
      )

    install(FILES ${gmo_file} DESTINATION ${LOCALEDIR}/${language}/LC_MESSAGES RENAME workrave.mo)
    set(gmo_files ${gmo_files} ${gmo_file})
  endif()
endforeach()
add_custom_target(translations ALL DEPENDS ${qm_files})

if (GETTEXT_FOUND)
  add_custom_target(nls ALL DEPENDS ${gmo_files})

  set(XGETTEXT_ARGS -cTRANSLATORS --qt --keyword=_ --keyword=N_ --keyword=Q_:1g --keyword=C_:1c,2 --keyword=NC_:1c,2 --keyword=translate:2 --keyword=translate:2,3c --keyword=g_dngettext:2,3 --from-code=UTF-8 --add-comments --msgid-bugs-address=https://github.com/rcaelers/workrave/issues)

  add_custom_command(
    OUTPUT ${POTFILE}
    COMMAND ${CMAKE_COMMAND} -E env GETTEXTDATADIRS=${CMAKE_SOURCE_DIR}/po ${GETTEXT_XGETTEXT_EXECUTABLE} ${XGETTEXT_ARGS} --files-from ${CMAKE_SOURCE_DIR}/po/POTFILES.in -o ${POTFILE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  macro(intltool_merge_xml IN OUT)
    add_custom_command(
      OUTPUT ${OUT}
      COMMAND ${CMAKE_COMMAND} -E env GETTEXTDATADIR=${CMAKE_SOURCE_DIR}/po ${GETTEXT_MSGFMT_EXECUTABLE}
        --xml
        -d "${CMAKE_SOURCE_DIR}/po"
        --template ${IN}
        -o ${OUT}
       DEPENDS ${IN} ${CMAKE_SOURCE_DIR}/po/POTFILES.in
    )
  endmacro()

  macro(intltool_merge_desktop IN OUT)
    add_custom_command(
      OUTPUT ${OUT}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
        --desktop
        --keyword=Name --keyword=Description
        -d "${CMAKE_SOURCE_DIR}/po"
        --template ${IN}
        -o ${OUT}
      DEPENDS ${IN}
      )
  endmacro()

  macro(intltool_nomerge_xml IN OUT)
    add_custom_command(
      OUTPUT ${OUT}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
        --xml
        -d "${CMAKE_SOURCE_DIR}/po"
        --template ${IN}
        -o ${OUT}
      DEPENDS ${IN}
      )
  endmacro()
endif()
