// NetworkConflictDialog.hh --- Network Conflict Dialog
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// $Id: NetworkConflictDialog.hh 1301 2007-08-30 21:25:47Z rcaelers $
//

#ifndef NETWORKCONFLICTDIACONFLICT_HH
#define NETWORKCONFLICTDIACONFLICT_HH

#include <stdio.h>

#include "preinclude.h"
#include <string>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/box.h>

#include "Hig.hh"

namespace Gtk
{
  class TextView;
}

// struct UIConfigConflict
// {
//   BreakId break_id;
//   string setting;

//   string local_value;
//   string remote_value;
// };

// typedef list
// //using namespace workrave;

class NetworkConflictDialog :
  public HigDialog
{
public:
  NetworkConflictDialog();
  ~NetworkConflictDialog();

  int run();


private:
  void init();

  void create_ui();
  void create_model();
  
  void on_response(int response);
  
private:
//   struct ModelColumns : public Gtk::TreeModelColumnRecord
//   {
//     Gtk::TreeModelColumn<std::string> name;
//     Gtk::TreeModelColumn<std::string> local_value;
//     Gtk::TreeModelColumn<std::string> remote_value;
//     Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf > > selection;

//     ModelColumns()
//     {
//       add(name);
//       add(local_value);
//       add(selection);
//       add(remote_value);
//     }
//   };

//   Gtk::TreeView *conflict_list;
//   Glib::RefPtr<Gtk::ListStore> conflict_store;
//   const ModelColumns conflict_columns;
//   Gtk::ScrolledWindow scrolled_window;

  Glib::RefPtr<Gdk::Pixbuf> pixbuf_left_arrow;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf_right_arrow;
};

#endif // NETWORKCONFLICTWINDOW_HH
