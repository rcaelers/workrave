if(WIN32)

  set(PERL_POSSIBLE_BIN_PATHS
    c:/strawberry/perl/bin
    d:/strawberry/perl/bin
  )
  get_filename_component(
    ActivePerl_CurrentVersion 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl;CurrentVersion]" 
    NAME)

  if (ActivePerl_CurrentVersion)
    set(PERL_POSSIBLE_BIN_PATHS ${PERL_POSSIBLE_BIN_PATHS}
      "c:/perl/bin" 
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\${ActivePerl_CurrentVersion}]/bin
      )
  endif (ActivePerl_CurrentVersion)
    
endif(WIN32)

FIND_PROGRAM(PERL_EXECUTABLE
  NAMES perl
  HINTS ${PERL_POSSIBLE_BIN_PATHS}
  )

#message($ENV{PATH})
#if (WIN32)
#  if (${PERL_EXECUTABLE} MATCHES strawberry)
#    get_filename_component(PERL_PATH ${PERL_EXECUTABLE} PATH)
#    if (NOT "$ENV{PATH}" MATCHES strawberry)
#      set($ENV{PATH} $ENV{PATH}:${PERL_PATH})
#    endif (NOT "$ENV{PATH}" MATCHES strawberry)
#  endif (${PERL_EXECUTABLE} MATCHES strawberry)
#endif (WIN32)
#message($ENV{PATH})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Perl DEFAULT_MSG PERL_EXECUTABLE)

MARK_AS_ADVANCED(PERL_EXECUTABLE)
