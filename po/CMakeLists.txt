######################################################################
## Translations
######################################################################

set(POTFILE ${CMAKE_CURRENT_SOURCE_DIR}/workrave.pot)
file(GLOB POFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)

add_custom_target(translations ALL)

if (GETTEXT_FOUND)
  add_custom_target(update-po)
endif()

foreach (po_file ${POFILES})
  get_filename_component(language ${po_file} NAME_WE)

  if (QT_LCONVERT_EXECUTABLE)
    set(ts_file ${CMAKE_CURRENT_BINARY_DIR}/workrave_${language}.ts)
    set(qm_file ${CMAKE_CURRENT_BINARY_DIR}/workrave_${language}.qm)

    add_custom_command(TARGET translations
        COMMAND ${QT_LCONVERT_EXECUTABLE} ARGS ${po_file} -o ${ts_file} -of ts
        DEPENDS ${po_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_command(TARGET translations
        COMMAND ${QT_LRELEASE_EXECUTABLE} ARGS ${ts_file} -qm ${qm_file}
        DEPENDS ${ts_file})

    install(FILES ${qm_file} DESTINATION ${DATADIR}/translations/)
  endif()

  if (GETTEXT_FOUND)
      add_custom_command(
        TARGET update-po
        POST_BUILD
        COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
          --quiet --update --no-wrap --backup=none -s ${po_file} ${POTFILE}
        VERBATIM
      )
  endif()

  if (GETTEXT_FOUND AND INTLTOOL_MERGE_FOUND)
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
      OUTPUT ${gmo_file}
      # COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --no-wrap --backup=none -s ${po_file} ${POTFILE}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${OPT_USE_FUZZY} -c -o ${gmo_file} ${po_file}
      DEPENDS ${POTFILE} ${po_file}
      )

    install(FILES ${gmo_file} DESTINATION ${LOCALEDIR}/${language}/LC_MESSAGES RENAME workrave.mo)
    set(gmo_files ${gmo_files} ${gmo_file})
  endif()

endforeach ()

if (GETTEXT_FOUND AND INTLTOOL_MERGE_FOUND)
  add_custom_target(nls ALL DEPENDS ${gmo_files})

  set(XGETTEXT_ARGS "-C --qt --keyword=_ --keyword=N_ --keyword=q_ --keyword=qc_:1,2c -keyword=translate:2 --keyword=translate:2,3c --from-code=utf-8")

  add_custom_command(
    OUTPUT ${POTFILE}
    COMMAND INTLTOOL_EXTRACT=${INTLTOOL_EXTRACT_EXECUTABLE} XGETTEXT=${GETTEXT_XGETTEXT_EXECUTABLE} XGETTEXT_ARGS=${XGETTEXT_ARGS} srcdir=. ${INTLTOOL_UPDATE_EXECUTABLE} --gettext-package workrave --pot -o ${POTFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
