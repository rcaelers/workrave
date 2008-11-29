SET(HAVE_DBUS 0)

IF(WIN32)
  SET(DBUS_DIR $ENV{GTKMM_BASEPATH}) ## CACHE PATH "Top-level directory where the gtkmm libraries are located")
  MARK_AS_ADVANCED(DBUS_DIR)
    
  SET(DBUS_INCLUDES
    ${DBUS_DIR}/include
    )
  
  SET(DBUS_LIBS
    ${DBUS_DIR}/lib/dbus.lib
    )
  SET(HAVE_DBUS 1)
  
ENDIF(WIN32)
