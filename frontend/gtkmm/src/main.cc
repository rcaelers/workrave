// main.cc --- Main
//
// Copyright (C) 2001, 2002, 2003, 2006 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#include "debug.hh"
#include <fstream>

#include "GUI.hh"
#ifdef WIN32
#include "dll_hell.h"
#endif

extern "C" int run(int argc, char **argv);

int
run(int argc, char **argv)
{
  GUI *gui = new GUI(argc, argv);

#ifdef WIN32
  dll_hell_check();
#endif

  gui->main();

  delete gui;

  return 0;
}


#if !defined(WIN32) || !defined(NDEBUG)
int
main(int argc, char **argv)
{
  int ret = run(argc, argv);
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
  char buf[1000];

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
