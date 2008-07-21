// NetworkLogDialog.hh --- Network Log Dialog
//
// Copyright (C) 2002, 2003, 2007 Raymond Penners <raymond@dotsphinx.com>
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
// $Id$
//

#ifndef NETWORKLOGDIALOG_HH
#define NETWORKLOGDIALOG_HH

#include <stdio.h>

#include "preinclude.h"
#include <string>

class TimeEntry;

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textbuffer.h>

using namespace std;

namespace Gtk
{
  class TextView;
}

#include "DistributionLogListener.hh"

using namespace workrave;

class NetworkLogDialog :
  public Gtk::Dialog,
  public DistributionLogListener
{
public:
  NetworkLogDialog();
  ~NetworkLogDialog();

  int run();

private:
  void init();
  void distribution_log(string msg);
  void on_response(int response);

  Gtk::TextView *text_view;
  Gtk::ScrolledWindow scrolled_window;
  Glib::RefPtr<Gtk::TextBuffer> text_buffer;
};

#endif // NETWORKLOGWINDOW_HH
