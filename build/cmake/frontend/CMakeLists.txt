project(Workrave)

set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../../common)
set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/../../backend)
set(FRONTEND_DIR ${CMAKE_SOURCE_DIR}/../../frontend)

######################################################################
## Workrave
######################################################################

set(FRONTEND_SOURCES 
    ${FRONTEND_DIR}/common/include/ISoundDriver.hh
    ${FRONTEND_DIR}/common/include/ITimeBar.hh
    ${FRONTEND_DIR}/common/include/ITimerBoxView.hh
    ${FRONTEND_DIR}/common/include/Orientation.hh
    ${FRONTEND_DIR}/common/include/Sound.hh
    ${FRONTEND_DIR}/common/include/SoundPlayer.hh
    ${FRONTEND_DIR}/common/include/System.hh
    ${FRONTEND_DIR}/common/include/Text.hh
    ${FRONTEND_DIR}/common/include/TimerBoxControl.hh
    ${FRONTEND_DIR}/common/include/credits.h
    ${FRONTEND_DIR}/common/src/GstSoundPlayer.cc
    ${FRONTEND_DIR}/common/src/GstSoundPlayer.hh
    ${FRONTEND_DIR}/common/src/SoundPlayer.cc
    ${FRONTEND_DIR}/common/src/System.cc
    ${FRONTEND_DIR}/common/src/Text.cc
    ${FRONTEND_DIR}/common/src/TimerBoxControl.cc
    ${FRONTEND_DIR}/gtkmm/src/AppletControl.cc
    ${FRONTEND_DIR}/gtkmm/src/AppletControl.hh
    ${FRONTEND_DIR}/gtkmm/src/AppletWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/AppletWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/BreakWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/BreakWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/DailyLimitWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/DailyLimitWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/DataConnector.cc
    ${FRONTEND_DIR}/gtkmm/src/DataConnector.hh
    ${FRONTEND_DIR}/gtkmm/src/EventButton.cc
    ${FRONTEND_DIR}/gtkmm/src/EventButton.hh
    ${FRONTEND_DIR}/gtkmm/src/EventImage.cc
    ${FRONTEND_DIR}/gtkmm/src/EventImage.hh
    ${FRONTEND_DIR}/gtkmm/src/EventLabel.cc
    ${FRONTEND_DIR}/gtkmm/src/EventLabel.hh
    ${FRONTEND_DIR}/gtkmm/src/Frame.cc
    ${FRONTEND_DIR}/gtkmm/src/Frame.hh
    ${FRONTEND_DIR}/gtkmm/src/GUI.cc
    ${FRONTEND_DIR}/gtkmm/src/GUI.hh
    ${FRONTEND_DIR}/gtkmm/src/GUIConfig.cc
    ${FRONTEND_DIR}/gtkmm/src/GUIConfig.hh
    ${FRONTEND_DIR}/gtkmm/src/GtkUtil.cc
    ${FRONTEND_DIR}/gtkmm/src/GtkUtil.hh
    ${FRONTEND_DIR}/gtkmm/src/HeadInfo.cc
    ${FRONTEND_DIR}/gtkmm/src/HeadInfo.hh
    ${FRONTEND_DIR}/gtkmm/src/Hig.cc
    ${FRONTEND_DIR}/gtkmm/src/Hig.hh
    ${FRONTEND_DIR}/gtkmm/src/IBreakWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/IPreludeWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/IconListCellRenderer.cc
    ${FRONTEND_DIR}/gtkmm/src/IconListCellRenderer.hh
    ${FRONTEND_DIR}/gtkmm/src/IconListNotebook.cc
    ${FRONTEND_DIR}/gtkmm/src/IconListNotebook.hh
    ${FRONTEND_DIR}/gtkmm/src/MainGtkMenu.hh
    ${FRONTEND_DIR}/gtkmm/src/MainGtkMenu.cc
    ${FRONTEND_DIR}/gtkmm/src/MainWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/MainWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/IMenu.hh
    ${FRONTEND_DIR}/gtkmm/src/Menus.cc
    ${FRONTEND_DIR}/gtkmm/src/Menus.hh
    ${FRONTEND_DIR}/gtkmm/src/MicroBreakWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/MicroBreakWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/PreferencesDialog.cc
    ${FRONTEND_DIR}/gtkmm/src/PreferencesDialog.hh
    ${FRONTEND_DIR}/gtkmm/src/PreludeWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/PreludeWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/RestBreakWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/RestBreakWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/StatusIcon.cc
    ${FRONTEND_DIR}/gtkmm/src/StatusIcon.hh
    ${FRONTEND_DIR}/gtkmm/src/Session.cc
    ${FRONTEND_DIR}/gtkmm/src/Session.hh
    ${FRONTEND_DIR}/gtkmm/src/TimeBar.cc
    ${FRONTEND_DIR}/gtkmm/src/TimeBar.hh
    ${FRONTEND_DIR}/gtkmm/src/TimeEntry.cc
    ${FRONTEND_DIR}/gtkmm/src/TimeEntry.hh
    ${FRONTEND_DIR}/gtkmm/src/TimerBoxGtkView.cc
    ${FRONTEND_DIR}/gtkmm/src/TimerBoxGtkView.hh
    ${FRONTEND_DIR}/gtkmm/src/TimerBoxPreferencePage.cc
    ${FRONTEND_DIR}/gtkmm/src/TimerBoxPreferencePage.hh
    ${FRONTEND_DIR}/gtkmm/src/TimerPreferencesPanel.cc
    ${FRONTEND_DIR}/gtkmm/src/TimerPreferencesPanel.hh
    ${FRONTEND_DIR}/gtkmm/src/WindowHints.cc
    ${FRONTEND_DIR}/gtkmm/src/WindowHints.hh
    ${FRONTEND_DIR}/gtkmm/src/macros.h
    ${FRONTEND_DIR}/gtkmm/src/main.cc
    ${FRONTEND_DIR}/gtkmm/src/preinclude.h
    ${FRONTEND_DIR}/plugin/distribution/gtkmm/src/NetworkJoinDialog.cc
    ${FRONTEND_DIR}/plugin/distribution/gtkmm/src/NetworkJoinDialog.hh
    ${FRONTEND_DIR}/plugin/distribution/gtkmm/src/NetworkLogDialog.cc
    ${FRONTEND_DIR}/plugin/distribution/gtkmm/src/NetworkLogDialog.hh
    ${FRONTEND_DIR}/plugin/distribution/gtkmm/src/NetworkPreferencePage.cc
    ${FRONTEND_DIR}/plugin/distribution/gtkmm/src/NetworkPreferencePage.hh
    ${FRONTEND_DIR}/plugin/statistics/gtkmm/src/StatisticsDialog.cc
    ${FRONTEND_DIR}/plugin/statistics/gtkmm/src/StatisticsDialog.hh
    ${FRONTEND_DIR}/gtkmm/src/eggsmclient.c
  )

if (APPLE)
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${FRONTEND_DIR}/common/src/osx/OSXSoundPlayer.cc
    ${FRONTEND_DIR}/common/src/osx/OSXSoundPlayer.hh
    ${FRONTEND_DIR}/common/src/osx/Sound.cc
    ${FRONTEND_DIR}/gtkmm/src/osx/AppController.h
    ${FRONTEND_DIR}/gtkmm/src/osx/OSXGtkMenu.cc
    ${FRONTEND_DIR}/gtkmm/src/osx/OSXGtkMenu.hh
    ${FRONTEND_DIR}/gtkmm/src/osx/OSXUtil.cc
    ${FRONTEND_DIR}/gtkmm/src/osx/OSXUtil.hh
    ${FRONTEND_DIR}/gtkmm/src/eggsmclient-osx.c
  )
endif (APPLE)
  
if (UNIX)
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${FRONTEND_DIR}/common/src/x11/Sound.cc
    ${FRONTEND_DIR}/gtkmm/src/X11SystrayAppletWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/X11SystrayAppletWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/unix/desktop-window.c
    ${FRONTEND_DIR}/gtkmm/src/unix/desktop-window.h
    ${FRONTEND_DIR}/gtkmm/src/gtktrayicon.c
    ${FRONTEND_DIR}/gtkmm/src/gtktrayicon.h
    ${FRONTEND_DIR}/gtkmm/src/eggdesktopfile.c
    ${FRONTEND_DIR}/gtkmm/src/eggsmclient-xsmp.c
  )
 add_definitions(-DEGG_SM_CLIENT_BACKEND_XSMP)
endif (UNIX)

if (WIN32)
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${FRONTEND_DIR}/common/src/win32/W32DirectSoundPlayer.cc
    ${FRONTEND_DIR}/common/src/win32/W32DirectSoundPlayer.hh
    ${FRONTEND_DIR}/common/src/win32/W32Mixer.cc
    ${FRONTEND_DIR}/common/src/win32/W32Mixer.hh
    ${FRONTEND_DIR}/common/src/win32/Sound.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/DesktopWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/DesktopWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/win32/W32AppletMenu.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/W32AppletMenu.hh
    ${FRONTEND_DIR}/gtkmm/src/win32/W32AppletWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/W32AppletWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/win32/W32Compat.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/W32Compat.hh
    ${FRONTEND_DIR}/gtkmm/src/win32/W32TrayMenu.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/W32TrayMenu.hh
    ${FRONTEND_DIR}/gtkmm/src/win32/W32StatusIcon.cc
    ${FRONTEND_DIR}/gtkmm/src/win32/W32StatusIcon.hh
    ${FRONTEND_DIR}/gtkmm/src/win32/dll_hell.c
    ${FRONTEND_DIR}/gtkmm/src/win32/dll_hell.h
    ${FRONTEND_DIR}/gtkmm/src/eggsmclient-win32.c
    ${CMAKE_BINARY_DIR}/workrave.rc
    )

    # ${FRONTEND_DIR}/common/src/win32/Sound.cc
    
  include_directories( 
    "${BACKEND_DIR}/src/win32"
    "${COMMON_DIR}/include/win32"
    "${COMMON_DIR}/win32/harpoon/include"
    "${FRONTEND_DIR}/common/src/win32"
    "${FRONTEND_DIR}/gtkmm/src/win32"
    "${FRONTEND_DIR}/applets/win32/include"
  )

  #ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/workrave_rc.o
  #                      COMMAND windres.exe
  #                          -o ${CMAKE_CURRENT_BINARY_DIR}/workrave_rc.o
  #                          -i ${CMAKE_CURRENT_BINARY_DIR}/workrave.rc
  #		         DEPENDS workrave.rc			    
  #                      )
  #	
  #set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
  #  ${CMAKE_CURRENT_BINARY_DIR}/workrave_rc.o
  #)
  
endif (WIN32)

if (HAVE_GNOME)
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${FRONTEND_DIR}/common/src/gnome/GnomeSoundPlayer.cc
    ${FRONTEND_DIR}/common/src/gnome/GnomeSoundPlayer.hh
    ${FRONTEND_DIR}/gtkmm/src/GnomeAppletMenu.cc
    ${FRONTEND_DIR}/gtkmm/src/GnomeAppletMenu.hh
    ${FRONTEND_DIR}/gtkmm/src/GnomeAppletWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/GnomeAppletWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/gnome_applet/WorkraveApplet.c
    ${FRONTEND_DIR}/gtkmm/src/gnome_applet/WorkraveApplet.h
  )
endif (HAVE_GNOME)

if (HAVE_KDE)
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${FRONTEND_DIR}/common/src/kde/KdeSoundPlayer.cc
    ${FRONTEND_DIR}/common/src/kde/KdeSoundPlayer.hh
    ${FRONTEND_DIR}/gtkmm/src/KdeAppletWindow.cc
    ${FRONTEND_DIR}/gtkmm/src/KdeAppletWindow.hh
    ${FRONTEND_DIR}/gtkmm/src/KdeWorkraveControl.cc
    ${FRONTEND_DIR}/gtkmm/src/KdeWorkraveControl.hh
    ${FRONTEND_DIR}/gtkmm/src/kde_applet/kworkraveapplet.h
    ${FRONTEND_DIR}/gtkmm/src/kde_applet/kworkravecontroliface.h
  )
endif(HAVE_KDE)

if (HAVE_DBUS)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/../../common/bin/dbusgen.py -l C++ -s ${FRONTEND_DIR}/gtkmm/src/workrave-gui.xml DBusGUI
  )
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
  )
  if (UNIX)
    set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
      ${FRONTEND_DIR}/gtkmm/src/eggsmclient-dbus.c
     )
  endif(UNIX)
  include_directories(${DBUS_INCLUDES})
  add_definitions(-DEGG_SM_CLIENT_BACKEND_DBUS)
endif (HAVE_DBUS)

if (HAVE_EXERCISES)
  set(FRONTEND_SOURCES ${FRONTEND_SOURCES}
    ${FRONTEND_DIR}/plugin/exercises/common/src/Exercise.cc
    ${FRONTEND_DIR}/plugin/exercises/common/src/Exercise.hh
    ${FRONTEND_DIR}/plugin/exercises/gtkmm/src/ExercisesDialog.cc
    ${FRONTEND_DIR}/plugin/exercises/gtkmm/src/ExercisesDialog.hh
    ${FRONTEND_DIR}/plugin/exercises/gtkmm/src/ExercisesPanel.cc
    ${FRONTEND_DIR}/plugin/exercises/gtkmm/src/ExercisesPanel.hh
  )
endif (HAVE_EXERCISES)

include_directories( 
  ${COMMON_DIR}/include
  ${BACKEND_DIR}/include
  ${FRONTEND_DIR}/include
  ${FRONTEND_DIR}/common/include
  ${FRONTEND_DIR}/gtkmm/src
  ${FRONTEND_DIR}/plugin/distribution/gtkmm/src
  ${FRONTEND_DIR}/plugin/exercises/common/src
  ${FRONTEND_DIR}/plugin/exercises/gtkmm/src
  ${FRONTEND_DIR}/plugin/statistics/gtkmm/src
  )
  
include_directories(${GTKMM_INCLUDES})
if (HAVE_DBUS)
  include_directories(${DBUS_INCLUDES})
endif(HAVE_DBUS)
if (HAVE_DIRECTX)
  include_directories(${DIRECTX_INCLUDES})
endif(HAVE_DIRECTX)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (HAVE_DIRECTX)
   link_directories(${DIRECTX_LIBS})
endif(HAVE_DIRECTX)

add_executable(workrave WIN32 ${FRONTEND_SOURCES})

target_link_libraries(workrave workrave-common)
target_link_libraries(workrave workrave-backend)
target_link_libraries(workrave ${GTKMM_LIBS})

if (HAVE_DBUS)
  target_link_libraries(workrave ${DBUS_LIBS})
endif(HAVE_DBUS)

if (HAVE_GNET)
  target_link_libraries(workrave ${GNET_LIBS})
endif(HAVE_GNET)

if (HAVE_DIRECTX)
   target_link_libraries(workrave dxerr dsound dxguid)
endif(HAVE_DIRECTX)

if (WIN32)
  target_link_libraries(workrave harpoon)
  target_link_libraries(workrave winmm shlwapi kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 wtsapi32)
endif(WIN32)

######################################################################
## Exercises
######################################################################

if (HAVE_EXERCISES)
  file(GLOB PO_FILES ${CMAKE_SOURCE_DIR}/../../po/*.po) 

  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml
      COMMAND ${INTLTOOL_MERGE_EXECUTABLE} -x -u -c ${CMAKE_CURRENT_BINARY_DIR}/intltool-merge-cache ${CMAKE_SOURCE_DIR}/../../po ${CMAKE_SOURCE_DIR}/../../frontend/plugin/exercises/common/share/exercises.xml.in ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml

      DEPENDS ${CMAKE_SOURCE_DIR}/../../frontend/plugin/exercises/common/share/exercises.xml.in
    )
endif (HAVE_EXERCISES)

######################################################################
## Translations
######################################################################

set(PO_SRC_DIR ${CMAKE_SOURCE_DIR}/../../po)
set(PO_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(COPYRIGHT_HOLDER "Workrave Development Team")
set(BUGS_ADDRESS "issues.workrave.org")
set(POT_FILE ${CMAKE_CURRENT_BINARY_DIR}/workrave.pot)

if (GETTEXT_FOUND)

  add_custom_target(nls-update-sources
    ${INTLTOOL_UPDATE_EXECUTABLE} -m
    WORKING_DIRECTORY ${PO_SRC_DIR}
  )

  add_custom_command(
    OUTPUT ${PO_BIN_DIR}/workrave.pot
    COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --directory=${CMAKE_SOURCE_DIR}/../../ --files-from ${PO_SRC_DIR}/POTFILES.in --keyword=_ --keyword=N_ -o ${POT_FILE} --copyright-holder=${COPYRIGHT_HOLDER} --msgid-bugs-address="${BUGS_ADDRESS}" --no-wrap --no-location
    DEPENDS POTFILES.in
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../../
  )

  #the string of all the locales must be separated by semicolons so that it is interpreted as a list
  #eg "it tr" becomes "it;tr"
  string(REPLACE " " ";" LANGLIST "${ALL_LINGUAS}")
  FOREACH(lang ${LANGLIST})
    #ADD_CUSTOM_TARGET(
    #  nls-update-${lang}.po
    #  ${GETTEXT_MSGMERGE_EXECUTABLE} ${PO_SRC_DIR}/${lang}.po ${PO_BIN_DIR}/workrave.pot -o ${lang}.po --no-wrap --sort-output
    #  DEPENDS ${PO_SRC_DIR}/${lang}.po
    #  WORKING_DIRECTORY ${PO_SRC_DIR}
    #)  

    FILE(MAKE_DIRECTORY "${PO_BIN_DIR}/${lang}/LC_MESSAGES")

    #Languages to allow fuzzy translations for.
    #For example to allow german and italian fuzzy translations:  -DLANG_FUZZY:STRING=de;it
    set(OPT_USE_FUZZY "")
    if(LANG_FUZZY)
      FOREACH(fuzz ${LANG_FUZZY})
        if("${fuzz}" STREQUAL "${lang}")
          set(OPT_USE_FUZZY --use-fuzzy)
          break()
        endif("${fuzz}" STREQUAL "${lang}")
      ENDFOREACH(fuzz)
    endif(LANG_FUZZY)

    ADD_CUSTOM_COMMAND(
      OUTPUT ${PO_BIN_DIR}/${lang}/LC_MESSAGES/workrave.mo
      COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${OPT_USE_FUZZY} -c -o ${PO_BIN_DIR}/${lang}/LC_MESSAGES/workrave.mo ${PO_SRC_DIR}/${lang}.po
      DEPENDS ${PO_SRC_DIR}/${lang}.po
    )

    #As seen in GUI::init_nls() if win32 workrave has gettext bind to lib/locale
    if(WIN32)
      set(LOCALE_DIR "lib/locale")
    else(WIN32)
      set(LOCALE_DIR "share/locale")
    endif(WIN32)

    INSTALL(FILES ${PO_BIN_DIR}/${lang}/LC_MESSAGES/workrave.mo
            DESTINATION ${LOCALE_DIR}/${lang}/LC_MESSAGES
    )

    SET(LANG_TARGETS ${LANG_TARGETS} ${PO_BIN_DIR}/${lang}/LC_MESSAGES/workrave.mo)
  ENDFOREACH(lang)

  if (HAVE_EXERCISES)
    set (EXERCISE_TARGETS ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml)
  endif (HAVE_EXERCISES)

  ADD_CUSTOM_TARGET(nls ALL
                    DEPENDS ${LANG_TARGETS} ${EXERCISE_TARGETS})

endif(GETTEXT_FOUND)
		    
######################################################################
## Installation
######################################################################

install(
DIRECTORY ${CMAKE_SOURCE_DIR}/../../frontend/common/share/images/
DESTINATION share/images
FILES_MATCHING
PATTERN "*.png"
PATTERN "*.ico"
PATTERN "*.svg"
)

install(TARGETS workrave
        RUNTIME DESTINATION lib
)

foreach(soundtheme ${SOUND_THEMES})
  file(GLOB SND_FILES  ${CMAKE_SOURCE_DIR}/../../frontend/common/share/sounds/${soundtheme}/*.wav
	  	       ${CMAKE_SOURCE_DIR}/../../frontend/common/share/sounds/${soundtheme}/soundtheme)

  install(FILES ${SND_FILES}
          DESTINATION share/sounds/${soundtheme}
  )
endforeach(soundtheme)

if (HAVE_EXERCISES)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exercises.xml
          DESTINATION share/exercises)

  file(GLOB EXERCISE_FILES ${CMAKE_SOURCE_DIR}/../../frontend/plugin/exercises/common/share/*.png)

  install(FILES ${EXERCISE_FILES}
          DESTINATION share/exercises)

endif(HAVE_EXERCISES)	   
