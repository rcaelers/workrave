// KdeAppletWindow.hh --- Main info Window
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef KDEAPPLETWINDOW_HH
#define KDEAPPLETWINDOW_HH

#include "config.h"

class KdeAppletWindow
{
public:
  static bool plug_window(int w);
  static bool get_size(int &size);
  static bool get_vertical(bool &vertical);
  static bool set_size(int width, int height);
};

#endif // KDEAPPLETWINDOW_HH
