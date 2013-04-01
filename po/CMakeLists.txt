######################################################################
## Translations
######################################################################

set(POTFILE ${CMAKE_CURRENT_SOURCE_DIR}/workrave.pot)

file(GLOB POFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
gettext_create_translations(${POTFILE} ALL ${POFILES})

set(XGETTEXT_ARGS "")
## --from-code=UTF-8 -C -kde -ci18n -k_ -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3")

add_custom_command(
  OUTPUT ${POTFILE}
  COMMAND INTLTOOL_EXTRACT=${INTLTOOL_EXTRACT_EXECUTABLE} XGETTEXT=${GETTEXT_XGETTEXT_EXECUTABLE} XGETTEXT_ARGS=${XGETTEXT_ARGS} srcdir=. ${INTLTOOL_UPDATE_EXECUTABLE} --gettext-package workrave --pot -o ${POTFILE}
  
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
