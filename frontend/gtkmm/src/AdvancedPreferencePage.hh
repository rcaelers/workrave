// AdvancedPreferencePage.hh --- Advanced preferences
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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
// See comments in AdvancedPreferencePage.cc
//

#ifndef ADVANCEDPREFERENCEPAGE_HH
#define ADVANCEDPREFERENCEPAGE_HH

#if defined(WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <string>

#include <gtkmm/liststore.h>
#include <gtkmm/box.h>

class Configurator;
namespace Gtk
{
  class Label;
  class Entry;
  class CheckButton;
  class SpinButton;
  class Button;
  class Notebook;
  class TextView;
}

class AdvancedPreferencePage :
  public Gtk::VBox
{
public:
  AdvancedPreferencePage();
  ~AdvancedPreferencePage();

private:

#if defined(WIN32)
  Gtk::CheckButton *forcebox;
  Gtk::CheckButton *nohooksbox;

  void forcebox_signal_toggled();
  bool forcebox_get_config();
  void nohooksbox_signal_toggled();
  bool nohooksbox_get_config();
#endif

  void init();
};

#endif // ADVANCEDPREFERENCEPAGE_HH
