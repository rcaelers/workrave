// main.cc --- Main
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <fstream>

#include "Control.hh"

extern "C" int run(int argc, char **argv);

int
run(int argc, char **argv)
{
  Control c;

  return c.main(argc, argv);
}

#if !defined(WIN32) || !defined(NDEBUG)
int
main(int argc, char **argv)
{
  // Don't show allocations that are allocated before main()
  Debug(make_all_allocations_invisible_except(NULL));

  std::ios::sync_with_stdio(false);

  // This will warn you when you are using header files that do not belong to the
  // shared libcwd object that you linked with.
  Debug( check_configuration() );

  // Turn on debug object `libcw_do'.
  Debug(libcw_do.on());
  Debug(dc::trace.on());
  
  // Turn on all debug channels that are off.
  //ForAllDebugChannels(
  //  if (!debugChannel.is_on())
  //    debugChannel.on();
  //);

#ifdef CWDEBUG
  std::ofstream file;
  file.open("workrave.log");
#endif

  // Set the ostream related with libcw_do to `file':  
  //Debug(libcw_do.set_ostream(&file));

  int ret = run(argc, argv);
  
  Dout(dc::trace, "=====");
  Debug(dc::malloc.on());
  Debug(dc::bfd.on());
  Debug(list_allocations_on(libcw_do));

#ifdef CWDEBUG
  file.close();
#endif
  
  return ret;
}

#else

#include <windows.h>

int WINAPI WinMain (HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance, 
                    PSTR szCmdLine, 
                    int iCmdShow) 
{
  char *argv[] = { szCmdLine };

  // InnoSetup: [...] requires that you add code to your application
  // which creates a mutex with the name you specify in this
  // directive.
  HANDLE mtx = CreateMutex(NULL, FALSE, "WorkraveMutex");
  if (mtx != NULL && GetLastError() != ERROR_ALREADY_EXISTS)
    {
      run(sizeof(argv)/sizeof(argv[0]), argv);
    }
  return (0);
}

#endif
