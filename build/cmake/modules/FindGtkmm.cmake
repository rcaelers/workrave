SET(HAVE_GTKMM 0)

IF(WIN32)
  SET(GTKMM_DIR $ENV{GTKMM_BASEPATH}) ## CACHE PATH "Top-level directory where the gtkmm libraries are located")
  MARK_AS_ADVANCED(GTKMM_DIR)
	
  IF (${CMAKE_GENERATOR} MATCHES "Visual Studio 9")
    SET(GTKMM_LIBS_VARIANT "-vc90")
  ELSEIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 8")
    SET(GTKMM_LIBS_VARIANT "-vc80")
  ELSEIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 10")
    SET(GTKMM_LIBS_VARIANT "-vc100")
  ELSE(${CMAKE_GENERATOR} MATCHES "Visual Studio 9")
    SET(GTKMM_LIBS_VARIANT "")
  ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 9")

  FIND_LIBRARY(ICONV_LIBS NAMES iconv
   PATHS
   ${GTKMM_DIR}/lib
  )
  
  EXECUTE_PROCESS(
        COMMAND ${GTKMM_DIR}/bin/pkg-config.exe --modversion glib-2.0
        OUTPUT_VARIABLE glib_version)

  IF (NOT ${glib_version} VERSION_LESS 2.24.0)
     SET(HAVE_GIO_NET 1)
  ENDIF(NOT ${glib_version} VERSION_LESS 2.24.0)

	
  SET(GTKMM_INCLUDES
    ${GTKMM_DIR}/include
    ${GTKMM_DIR}/include/atk-1.0
    ${GTKMM_DIR}/include/atkmm-1.6
    ${GTKMM_DIR}/include/cairo
    ${GTKMM_DIR}/include/cairomm-1.0
    ${GTKMM_DIR}/include/gdkmm-2.4
    ${GTKMM_DIR}/include/giomm-2.4
    ${GTKMM_DIR}/include/glib-2.0
    ${GTKMM_DIR}/include/glibmm-2.4
    ${GTKMM_DIR}/include/gtk-2.0
    ${GTKMM_DIR}/include/gtkmm-2.4
    ${GTKMM_DIR}/include/libglade-2.0
    ${GTKMM_DIR}/include/libglademm-2.4
    ${GTKMM_DIR}/include/libxml++-2.6
    ${GTKMM_DIR}/include/libxml2
    ${GTKMM_DIR}/include/pango-1.0
    ${GTKMM_DIR}/include/pangomm-1.4
    ${GTKMM_DIR}/include/sigc++-2.0
    ${GTKMM_DIR}/include/freetype2
    ${GTKMM_DIR}/include/gdk-pixbuf-2.0
    ${GTKMM_DIR}/lib/gdkmm-2.4/include
    ${GTKMM_DIR}/lib/giomm-2.4/include
    ${GTKMM_DIR}/lib/glib-2.0/include
    ${GTKMM_DIR}/lib/glibmm-2.4/include
    ${GTKMM_DIR}/lib/gtk-2.0/include
    ${GTKMM_DIR}/lib/gtkmm-2.4/include
    ${GTKMM_DIR}/lib/libglademm-2.4/include
    ${GTKMM_DIR}/lib/pangomm-1.4/include
    ${GTKMM_DIR}/lib/libxml++-2.6/include
    ${GTKMM_DIR}/lib/sigc++-2.0/include
    ${GTKMM_DIR}/lib/pangomm-1.4/include
    )
  
  SET(GTKMM_LIBS
    optimized ${GTKMM_DIR}/lib/atkmm${GTKMM_LIBS_VARIANT}-1_6.lib
    debug ${GTKMM_DIR}/lib/atkmm${GTKMM_LIBS_VARIANT}-d-1_6.lib
    optimized ${GTKMM_DIR}/lib/cairomm${GTKMM_LIBS_VARIANT}-1_0.lib
    debug ${GTKMM_DIR}/lib/cairomm${GTKMM_LIBS_VARIANT}-d-1_0.lib
    optimized ${GTKMM_DIR}/lib/gdkmm${GTKMM_LIBS_VARIANT}-2_4.lib
    debug ${GTKMM_DIR}/lib/gdkmm${GTKMM_LIBS_VARIANT}-d-2_4.lib
    optimized ${GTKMM_DIR}/lib/giomm${GTKMM_LIBS_VARIANT}-2_4.lib
    debug ${GTKMM_DIR}/lib/giomm${GTKMM_LIBS_VARIANT}-d-2_4.lib
    optimized ${GTKMM_DIR}/lib/glademm${GTKMM_LIBS_VARIANT}-2_4.lib
    debug ${GTKMM_DIR}/lib/glademm${GTKMM_LIBS_VARIANT}-d-2_4.lib
    optimized ${GTKMM_DIR}/lib/glibmm${GTKMM_LIBS_VARIANT}-2_4.lib
    debug ${GTKMM_DIR}/lib/glibmm${GTKMM_LIBS_VARIANT}-d-2_4.lib
    optimized ${GTKMM_DIR}/lib/gtkmm${GTKMM_LIBS_VARIANT}-2_4.lib
    debug ${GTKMM_DIR}/lib/gtkmm${GTKMM_LIBS_VARIANT}-d-2_4.lib
    optimized ${GTKMM_DIR}/lib/pangomm${GTKMM_LIBS_VARIANT}-1_4.lib
    debug ${GTKMM_DIR}/lib/pangomm${GTKMM_LIBS_VARIANT}-d-1_4.lib
    optimized ${GTKMM_DIR}/lib/sigc${GTKMM_LIBS_VARIANT}-2_0.lib
    debug ${GTKMM_DIR}/lib/sigc${GTKMM_LIBS_VARIANT}-d-2_0.lib
    optimized ${GTKMM_DIR}/lib/xml++${GTKMM_LIBS_VARIANT}-2_6.lib
    debug ${GTKMM_DIR}/lib/xml++${GTKMM_LIBS_VARIANT}-d-2_6.lib
    ${GTKMM_DIR}/lib/atk-1.0.lib
    ${GTKMM_DIR}/lib/cairo.lib
    ${GTKMM_DIR}/lib/gdk-win32-2.0.lib
    ${GTKMM_DIR}/lib/gdk_pixbuf-2.0.lib
    ${GTKMM_DIR}/lib/gio-2.0.lib
    ${GTKMM_DIR}/lib/glade-2.0.lib
    ${GTKMM_DIR}/lib/glib-2.0.lib
    ${GTKMM_DIR}/lib/gthread-2.0.lib
    ${GTKMM_DIR}/lib/gmodule-2.0.lib
    ${GTKMM_DIR}/lib/gobject-2.0.lib
    ${GTKMM_DIR}/lib/gtk-win32-2.0.lib
    ${GTKMM_DIR}/lib/libxml2.lib
    ${GTKMM_DIR}/lib/pango-1.0.lib
    ${GTKMM_DIR}/lib/pangocairo-1.0.lib
    ${GTKMM_DIR}/lib/pangowin32-1.0.lib
    ${GTKMM_DIR}/lib/intl.lib
    ${ICONV_LIBS}
    )

  SET(GTKMM_DEBUG_LIBS
    ${GTKMM_DIR}/lib/gtk-win32-2.0.lib
    ${GTKMM_DIR}/lib/libxml2.lib
    ${GTKMM_DIR}/lib/gdk-win32-2.0.lib
    ${GTKMM_DIR}/lib/atk-1.0.lib
    ${GTKMM_DIR}/lib/gdk_pixbuf-2.0.lib
    ${GTKMM_DIR}/lib/pangowin32-1.0.lib
    ${GTKMM_DIR}/lib/pangocairo-1.0.lib
    ${GTKMM_DIR}/lib/pango-1.0.lib
    ${GTKMM_DIR}/lib/cairo.lib
    ${GTKMM_DIR}/lib/gio-2.0.lib
    ${GTKMM_DIR}/lib/gobject-2.0.lib
    ${GTKMM_DIR}/lib/gmodule-2.0.lib
    ${GTKMM_DIR}/lib/glib-2.0.lib
    ${GTKMM_DIR}/lib/gthread-2.0.lib
    ${GTKMM_DIR}/lib/intl.lib
    ${ICONV_LIBS}
    )

    SET(HAVE_GTKMM 1)
  
ENDIF(WIN32)
