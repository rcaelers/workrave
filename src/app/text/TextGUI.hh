// TextGUI.hh --- Text GUI Creator
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-01-06 20:37:06 robc>
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

#ifndef TEXTGUI_HH
#define TEXTGUI_HH

#include "GUIInterface.hh"

class ControlInterface;

class TextGUI
{
public:
  static GUIInterface *create(ControlInterface *c, int argc, char **argv);
};

#endif // TEXTGUI_HH
