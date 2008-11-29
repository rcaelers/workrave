SET(HAVE_GLIB 0)

IF(WIN32)
  SET(GLIB_DIR $ENV{GTKMM_BASEPATH}) ## CACHE PATH "Top-level directory where the gtkmm libraries are located")
  MARK_AS_ADVANCED(GLIB_DIR)
	
  IF (${CMAKE_GENERATOR} MATCHES "Visual Studio 9")
    SET(GLIB_LIBS_VARIANT "-vc90")
  ELSEIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 8")
    SET(GLIB_LIBS_VARIANT "-vc80")
  ELSE(${CMAKE_GENERATOR} MATCHES "Visual Studio 9")
    SET(GLIB_LIBS_VARIANT "")
  ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 9")
    
  SET(GLIB_INCLUDES
    ${GLIB_DIR}/include
    ${GLIB_DIR}/include/glib-2.0
    ${GLIB_DIR}/lib/glib-2.0/include
    )
  
  SET(GLIB_LIBS
    ${GLIB_DIR}/lib/gio-2.0.lib
    ${GLIB_DIR}/lib/gobject-2.0.lib
    ${GLIB_DIR}/lib/gmodule-2.0.lib
    ${GLIB_DIR}/lib/glib-2.0.lib
    ${GLIB_DIR}/lib/intl.lib
    ${GLIB_DIR}/lib/iconv.lib
    )
  SET(HAVE_GLIB 1)
  
ENDIF(WIN32)
