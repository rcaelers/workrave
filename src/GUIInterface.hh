// GUIInterface.hh --- C++ interface that must be implemented by all GUIs
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-13 12:20:56 robc>
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

#ifndef GUIINTERFACE_HH
#define GUIINTERFACE_HH

#include <string>

class Configurator;

class GUIInterface
{
public:
  // Virtual destructor..
  virtual ~GUIInterface() {}
  
  //! Activate GUI.
  virtual void run() = 0;

  //! Returns the configurator.
  virtual Configurator *get_configurator() = 0;
};

#endif // GUIINTERFACE_HH
