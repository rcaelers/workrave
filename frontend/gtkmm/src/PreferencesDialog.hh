// PreferencesDialog.hh --- Preferences Dialog
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2010, 2011 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef PREFERENCESDIALOG_HH
#define PREFERENCESDIALOG_HH

#include <stdio.h>

#include <vector>

#include "preinclude.h"
#include "Hig.hh"
#include "IconListNotebook.hh"
#include "ICore.hh"

#include "SoundPlayer.hh"

#include <gtkmm.h>

class TimeEntry;
class DataConnector;

namespace Gtk
{
  class ComboBox;
  class ComboBox;
  class FileChooserButton;
  class HScale;
  class FileFilter;
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
  Gtk::Widget *create_sounds_page();
#ifdef HAVE_DISTRIBUTION
  Gtk::Widget *create_network_page();
#endif
  Gtk::Widget *create_applet_page();
  Gtk::Widget *create_mainwindow_page();
  bool on_focus_in_event(GdkEventFocus *event);
  bool on_focus_out_event(GdkEventFocus *event);

  void on_sound_changed();
  void on_block_changed();

  Gtk::ComboBoxText *sound_button;
  Gtk::ComboBoxText *block_button;
  Gtk::ComboBoxText *sound_theme_button;
  
  // Mode before focus in.
  OperationMode mode;
  IconListNotebook notebook;

#if defined(HAVE_LANGUAGE_SELECTION)
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

  class SoundModel : public Gtk::TreeModel::ColumnRecord
  {
  public:
    SoundModel()
    { add(enabled); add(description); add(selectable); add(label); add(event); }

    Gtk::TreeModelColumn<bool> enabled;
    Gtk::TreeModelColumn<Glib::ustring> description;
    Gtk::TreeModelColumn<bool> selectable;
    Gtk::TreeModelColumn<Glib::ustring> label;
    Gtk::TreeModelColumn<int> event;
  };

  DataConnector *connector;
  std::vector<SoundPlayer::Theme> sound_themes;
  Gtk::TreeView sound_treeview;
  SoundModel sound_model;
  Glib::RefPtr<Gtk::ListStore> sound_store;
  Gtk::CellRendererToggle sound_enabled_cellrenderer;
  Gtk::CellRendererText sound_event_cellrenderer;
  Gtk::HScale *sound_volume_scale;
  Gtk::Button *sound_play_button;
  int inhibit_events;
  Gtk::CheckButton *mute_cb;
  
  Gtk::FileChooserButton *fsbutton;
#ifdef HAVE_GTK3
  Glib::RefPtr<Gtk::FileFilter> filefilter;
#else
  Gtk::FileFilter *filefilter;
#endif  
  std::string fsbutton_filename;
  Gtk::CheckButton *trayicon_cb;
  
  void on_sound_enabled(const Glib::ustring& path_stringxo);
  void on_sound_play();
  void on_sound_filechooser_play();
  void on_sound_filechooser_select();
  void on_sound_events_changed();
  void on_sound_theme_changed();
  void update_theme_selection();
  void update_senstives();
  
#if defined(PLATFORM_OS_WIN32)
  Gtk::CheckButton *autostart_cb;
  void on_autostart_toggled();
#endif
};

#endif // PREFERENCESWINDOW_HH
