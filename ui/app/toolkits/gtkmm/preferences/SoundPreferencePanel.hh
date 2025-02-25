// Copyright (C) 2002-  2012 Rob Caelers <robc@krandor.nl>
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

#ifndef SOUNDPREFERENCEPANEL_HH
#define SOUNDPREFERENCEPANEL_HH

#include <string>

#include "utils/Signals.hh"
#include "ui/IApplicationContext.hh"

#include <gtkmm.h>

class DataConnector;

class SoundPreferencePanel
  : public Gtk::VBox
  , public workrave::utils::Trackable
{
public:
  explicit SoundPreferencePanel(std::shared_ptr<IApplicationContext> app);
  ~SoundPreferencePanel() override;

  void create_panel();

private:
  void on_sound_changed();
  void on_sound_enabled(const Glib::ustring &path_stringxo);
  void on_sound_play();
  void on_sound_filechooser_play();
  void on_sound_filechooser_select();
  void on_sound_events_changed();
  void on_sound_theme_changed();
  void update_sound_theme_selection();
  void update_senstives();

private:
  std::shared_ptr<DataConnector> connector;
  SoundTheme::Ptr sound_theme;

  class SoundModel : public Gtk::TreeModel::ColumnRecord
  {
  public:
    SoundModel()
    {
      add(enabled);
      add(description);
      add(selectable);
      add(label);
    }

    Gtk::TreeModelColumn<bool> enabled;
    Gtk::TreeModelColumn<Glib::ustring> description;
    Gtk::TreeModelColumn<bool> selectable;
    Gtk::TreeModelColumn<Glib::ustring> label;
  };

  Gtk::ComboBoxText *sound_button{nullptr};
  Gtk::ComboBoxText *sound_theme_button{nullptr};

  int inhibit_events{0};
  Gtk::TreeView sound_treeview;
  SoundModel sound_model;
  Glib::RefPtr<Gtk::ListStore> sound_store;
  Gtk::CellRendererToggle sound_enabled_cellrenderer;
  Gtk::CellRendererText sound_event_cellrenderer;
  Gtk::HScale *sound_volume_scale{nullptr};
  Gtk::Button *sound_play_button{nullptr};
  Gtk::CheckButton *mute_cb{nullptr};
  Gtk::FileChooserButton *fsbutton{nullptr};
  Glib::RefPtr<Gtk::FileFilter> filefilter;
  std::string fsbutton_filename;
};

#endif // SOUNDPREFERENCEPANEL_HH
