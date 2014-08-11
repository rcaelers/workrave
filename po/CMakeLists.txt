######################################################################
## Translations
######################################################################

if (GETTEXT_FOUND AND INTLTOOL_MERGE_FOUND)

  set(POTFILE ${CMAKE_CURRENT_SOURCE_DIR}/workrave.pot)
  file(GLOB POFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
  
  foreach (po_file ${POFILES})
    get_filename_component(language ${po_file} NAME_WE)
    set(gmo_file ${CMAKE_CURRENT_BINARY_DIR}/${language}.gmo)
  
      #Languages to allow fuzzy translations for.
      #For example to allow german and italian fuzzy translations:  -Dlang_fuzzy:STRING=de;it
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
      COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --no-wrap --sort-output --backup=none -s ${po_file} ${POTFILE}
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${OPT_USE_FUZZY} -c -o ${gmo_file} ${po_file}
      DEPENDS ${POTFILE} ${po_file}
      )
    
    install(FILES ${gmo_file} DESTINATION ${LOCALEDIR}/${language}/LC_MESSAGES RENAME workrave.mo)
    set(gmo_files ${gmo_files} ${gmo_file})
  endforeach ()
  
  add_custom_target(nls ALL DEPENDS ${gmo_files})
  
  set(XGETTEXT_ARGS "")
  ## --from-code=UTF-8 -C -kde -ci18n -k_ -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3")
  
  add_custom_command(
    OUTPUT ${POTFILE}
    COMMAND INTLTOOL_EXTRACT=${INTLTOOL_EXTRACT_EXECUTABLE} XGETTEXT=${GETTEXT_XGETTEXT_EXECUTABLE} XGETTEXT_ARGS=${XGETTEXT_ARGS} srcdir=. ${INTLTOOL_UPDATE_EXECUTABLE} --gettext-package workrave --pot -o ${POTFILE}
    
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()