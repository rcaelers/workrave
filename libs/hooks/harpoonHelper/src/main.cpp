// main.cc --- Main
//
// Copyright (C) 2010 Rob Caelers 
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>
#include <stdio.h>
#include <windows.h>


#include "HarpoonHelper.h"
#include "Debug.h" 

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PSTR szCmdLine,
                    int iCmdShow)
{
#ifndef NDEBUG 
  Debug::init();
#endif

  TRACE_ENTER_MSG("WinMain", szCmdLine);

  HarpoonHelper *h = new HarpoonHelper(szCmdLine);
  h->init(hInstance);
  h->run();

  TRACE_EXIT();
  return (0);
}
