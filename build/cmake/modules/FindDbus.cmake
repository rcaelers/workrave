set(HAVE_DBUS 0)

# will be empty if not WIN32
file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _progFiles)

find_path(DBUS_HEADER_INCLUDE_DIR dbus/dbus.h
   PATHS
   ${_progFiles}/dbus/include
   PATH_SUFFIXES dbus-1.0
)

find_library(DBUS_LIBS NAMES dbus-1
   PATHS
   ${_progFiles}/dbus/lib
)

find_library(DBUS_DEBUG_LIBS NAMES dbus-1d
   PATHS
   ${_progFiles}/dbus/lib
)

if (DBUS_LIBS)

   get_filename_component(_dbusLibPath ${DBUS_LIBS} PATH)

   if (DBUS_DEBUG_LIBS)
      set(DBUS_LIBS
         optimized ${DBUS_LIBS}
         debug ${DBUS_DEBUG_LIBS}
      )
   endif (DBUS_DEBUG_LIBS)
   

   
   find_path(DBUS_LIB_INCLUDE_DIR dbus/dbus-arch-deps.h
      PATHS
      ${_dbusLibPath}
      ${_progFiles}/dbus/include
      PATH_SUFFIXES dbus-1.0/include
   )
endif (DBUS_LIBS)

set(DBUS_INCLUDES ${DBUS_HEADER_INCLUDE_DIR} ${DBUS_LIB_INCLUDE_DIR})

if (DBUS_INCLUDE_DIR AND DBUS_LIBS)
   set(DBUS_FOUND TRUE)
else (DBUS_INCLUDE_DIR AND DBUS_LIBS)
   set(DBUS_FOUND FALSE)
endif (DBUS_INCLUDE_DIR AND DBUS_LIBS)
