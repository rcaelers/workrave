// GUI.cc
//
// Copyright (C) 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "GUI.hh"

int run(int argc, char *argv[])
{
  GUI gui(argc, argv);
  return gui.exec();
}


#ifdef PLATFORM_OS_WIN32
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
