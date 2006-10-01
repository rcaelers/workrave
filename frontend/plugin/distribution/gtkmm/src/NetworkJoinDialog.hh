// NetworkJoinDialog.hh --- NetworkJoin Dialog
//
// Copyright (C) 2002, 2003, 2004, 2006 Raymond Penners <raymond@dotsphinx.com>
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

#include <string>
#include <stdio.h>

#include "preinclude.h"
#include "Hig.hh"

#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>

class IDistributionManager;
class TimeEntry;

class NetworkJoinDialog : public HigDialog
{
public:  
  NetworkJoinDialog();
  ~NetworkJoinDialog();

  std::string get_connect_url();

private:
  Gtk::Entry host_entry;
  Gtk::SpinButton port_entry;
};

#endif // NETWORKJOINWINDOW_HH
