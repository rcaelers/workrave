// NetworkJoinDialog.hh --- NetworkJoin Dialog
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef NETWORKJOINDIALOG_HH
#define NETWORKJOINDIALOG_HH

#include <stdio.h>

#include "preinclude.h"

class ControlInterface;
class TimeEntry;

#include <gtkmm.h>

class NetworkJoinDialog : public Gtk::Dialog
{
public:  
  NetworkJoinDialog();
  ~NetworkJoinDialog();

  int run();
  
private:
  void init();

  Gtk::Entry *host_entry;
  Gtk::SpinButton *port_entry;
  Gtk::CheckButton *startup_button;
};

#endif // NETWORKJOINWINDOW_HH
