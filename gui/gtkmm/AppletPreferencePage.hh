// AppletPreferencesPanel.hh --- Preferences widgets for a timer
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#ifndef APPLETPREFERENCEPAGE_HH
#define APPLETPREFERENCEPAGE_HH

#include <stdio.h>
#include <string>

#include "preinclude.h"

class Configurator;

#include <gtkmm.h>

class AppletPreferencePage
  : public Gtk::HBox
{
public:  
  AppletPreferencePage();
  ~AppletPreferencePage();
  
private:
  //void init_page_values();
  void create_model();
};

#endif // APPLETPREFERENCEPAGE_HH
