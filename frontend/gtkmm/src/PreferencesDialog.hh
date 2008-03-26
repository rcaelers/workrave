// PreferencesDialog.hh --- Preferences Dialog
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef PREFERENCESDIALOG_HH
#define PREFERENCESDIALOG_HH

#include <stdio.h>

#include <vector>

#include "preinclude.h"
#include "Hig.hh"
#include "IconListNotebook.hh"
#include "ICore.hh"

#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

class TimeEntry;
namespace Gtk
{
  class OptionMenu;
  class ComboBox;
}

using namespace workrave;

class PreferencesDialog : public HigDialog
{
public:
  PreferencesDialog();
  ~PreferencesDialog();

  int run();

private:
  void add_page(const char *label, const char *image, Gtk::Widget &widget);
  Gtk::Widget *create_gui_page();
  Gtk::Widget *create_timer_page();
#ifdef HAVE_DISTRIBUTION
  Gtk::Widget *create_network_page();
#endif
  Gtk::Widget *create_applet_page();
  Gtk::Widget *create_mainwindow_page();
  bool on_focus_in_event(GdkEventFocus *event);
  bool on_focus_out_event(GdkEventFocus *event);

  void on_sound_changed();
  void on_block_changed();
  void on_language_changed();

  Gtk::OptionMenu *sound_button;
  Gtk::OptionMenu *block_button;

  // Mode before focus in.
  OperationMode mode;
  IconListNotebook notebook;

#if defined(PLATFORM_OS_WIN32)
  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(current); add(native); add(enabled); add(code);  }

    Gtk::TreeModelColumn<bool> enabled;
    Gtk::TreeModelColumn<Glib::ustring> code;
    Gtk::TreeModelColumn<Glib::ustring> native;
    Gtk::TreeModelColumn<Glib::ustring> current;
  };

  void on_native_cell_data(const Gtk::TreeModel::const_iterator& iter);
  void on_current_cell_data(const Gtk::TreeModel::const_iterator& iter);
  int on_cell_data_compare(const Gtk::TreeModel::iterator& iter1,
                           const Gtk::TreeModel::iterator& iter2);
  
  Gtk::ComboBox languages_combo;
  ModelColumns languages_columns;
  Glib::RefPtr<Gtk::ListStore> languages_model;
  Gtk::CellRendererText native_cellrenderer;
  Gtk::CellRendererText current_cellrenderer;
#endif
};

#endif // PREFERENCESWINDOW_HH
