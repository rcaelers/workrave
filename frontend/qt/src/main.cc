// GUI.cc
//
// Copyright (C) 2006 Raymond Penners <raymond@dotsphinx.com>
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
// $Id: IniConfigurator.hh 558 2006-02-23 19:42:12Z rcaelers $
//

#include "GUI.hh"

int run(int argc, char *argv[])
{
  GUI gui(argc, argv);
  return gui.exec();
}

     
#ifdef WIN32
#include <windows.h>
int APIENTRY
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
  return run(1, &lpCmdLine);
}

#else

int main(int argc, char **argv )
{
  return run(argc, argv);
}

#endif
