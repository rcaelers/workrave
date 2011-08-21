// GenericDBusAppletMenu.cc --- Menus using IndicatorApplet+
//
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <string>

#include "GenericDBusAppletMenu.hh"
#include "GenericDBusAppletWindow.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "Util.hh"

using namespace std;


//! Constructor.
GenericDBusAppletMenu::GenericDBusAppletMenu(GenericDBusAppletWindow *applet_window)
  :applet_window(applet_window)
{
}

//! Destructor.
GenericDBusAppletMenu::~GenericDBusAppletMenu()
{
}

void
GenericDBusAppletMenu::init()
{
}

void
GenericDBusAppletMenu::add_accel(Gtk::Window &window)
{
  (void) window;
}

void
GenericDBusAppletMenu::popup(const guint button, const guint activate_time)
{
  (void) button;
  (void) activate_time;
}

void
GenericDBusAppletMenu::resync(OperationMode mode, UsageMode usage, bool show_log)
{
}

