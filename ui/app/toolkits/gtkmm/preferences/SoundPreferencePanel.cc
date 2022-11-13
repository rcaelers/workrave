// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include <utility>

#  include "config.h"
#endif

#include "SoundPreferencePanel.hh"

#include "commonui/nls.h"
#include "ui/GUIConfig.hh"

#include "DataConnector.hh"
#include "Hig.hh"
#include "Ui.hh"

#include "debug.hh"

using namespace workrave;
using namespace workrave::utils;

SoundPreferencePanel::SoundPreferencePanel(std::shared_ptr<IApplicationContext> app)
  : Gtk::VBox(false, 6)
  , connector(std::make_shared<DataConnector>(app))
  , sound_theme(app->get_sound_theme())
{
  TRACE_ENTRY();
  create_panel();
}

SoundPreferencePanel::~SoundPreferencePanel()
{
  TRACE_ENTRY();
}

void
SoundPreferencePanel::create_panel()
{
  // Options
  HigCategoryPanel *hig = Gtk::manage(new HigCategoryPanel(_("Sound Options")));
  pack_start(*hig, false, false, 0);

  // Sound types
  sound_button = Gtk::manage(new Gtk::ComboBoxText());
  sound_button->append(_("No sounds"));
  sound_button->append(_("Play sounds using sound card"));
  int idx = 0;
  if (!sound_theme->sound_enabled()())
    {
      idx = 0;
    }
  else
    {
      idx = 1;
    }
  sound_button->set_active(idx);
  sound_button->signal_changed().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_changed));

  if (sound_theme->capability(workrave::audio::SoundCapability::VOLUME))
    {
      // Volume
      sound_volume_scale = Gtk::manage(new Gtk::HScale(0.0, 100.0, 0.0));
      sound_volume_scale->set_increments(1.0, 5.0);
      connector->connect(sound_theme->sound_volume(), dc::wrap(sound_volume_scale->get_adjustment()));

      hig->add_label(_("Volume:"), *sound_volume_scale, true, true);
    }

  hig->add_label(_("Sound:"), *sound_button);

  if (sound_theme->capability(workrave::audio::SoundCapability::MUTE))
    {
      // Volume
      mute_cb = Gtk::manage(new Gtk::CheckButton(_("Mute sounds during rest break and daily limit")));

      connector->connect(sound_theme->sound_mute(), dc::wrap(mute_cb));

      hig->add_widget(*mute_cb, true, true);
    }

  // Sound themes
  hig = Gtk::manage(new HigCategoryPanel(_("Sound Events"), true));
  pack_start(*hig, true, true, 0);

  sound_theme_button = Gtk::manage(new Gtk::ComboBoxText());

  update_sound_theme_selection();

  sound_theme_button->signal_changed().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_theme_changed));
  hig->add_label(_("Sound Theme:"), *sound_theme_button);

  sound_store = Gtk::ListStore::create(sound_model);
  sound_treeview.set_model(sound_store);

  SoundTheme::ThemeInfo::Ptr active_theme = sound_theme->get_active_theme();

  for (SoundTheme::SoundInfo snd: active_theme->sounds)
    {
      bool sound_enabled = sound_theme->sound_event_enabled(snd.event)();

      Gtk::TreeModel::iterator iter = sound_store->append();
      Gtk::TreeModel::Row row = *iter;

      row[sound_model.enabled] = sound_enabled;
      row[sound_model.selectable] = true;
      row[sound_model.description] = Ui::get_sound_event_name(snd.event);
      row[sound_model.label] = sound_theme->sound_event_to_id(snd.event);
    }

  sound_treeview.set_rules_hint();
  sound_treeview.set_search_column(sound_model.description.index());

  int cols_count = sound_treeview.append_column_editable(_("Play"), sound_model.enabled);
  Gtk::TreeViewColumn *column = sound_treeview.get_column(cols_count - 1);

  auto *cell = dynamic_cast<Gtk::CellRendererToggle *>(sound_treeview.get_column_cell_renderer(cols_count - 1));

  cols_count = sound_treeview.append_column(_("Event"), sound_model.description);
  column = sound_treeview.get_column(cols_count - 1);
  column->set_fixed_width(40);

  Gtk::ScrolledWindow *sound_scroll = Gtk::manage(new Gtk::ScrolledWindow());
  sound_scroll->add(sound_treeview);
  sound_scroll->set_size_request(-1, 200);
  sound_treeview.set_size_request(-1, 200);

  hig->add_widget(*sound_scroll, true, true);

  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 6));

  sound_play_button = Gtk::manage(new Gtk::Button(_("Play")));
  hbox->pack_start(*sound_play_button, false, false, 0);

  fsbutton = Gtk::manage(new Gtk::FileChooserButton(_("Choose a sound"), Gtk::FILE_CHOOSER_ACTION_OPEN));

  hbox->pack_start(*fsbutton, true, true, 0);

  filefilter = Gtk::FileFilter::create();

  filefilter->set_name(_("Wavefiles"));
#if defined(PLATFORM_OS_WINDOWS)
  filefilter->add_pattern("*.wav");
#else
  filefilter->add_mime_type("audio/x-wav");
#endif
  fsbutton->add_filter(filefilter);

  hig->add_widget(*hbox);

  Gtk::HBox *selector_hbox = Gtk::manage(new Gtk::HBox(false, 0));
  Gtk::Button *selector_playbutton = Gtk::manage(new Gtk::Button(_("Play")));

  selector_hbox->pack_end(*selector_playbutton, false, false, 0);
  selector_playbutton->show();
  fsbutton->set_extra_widget(*selector_hbox);

  cell->signal_toggled().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_enabled));

  sound_play_button->signal_clicked().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_play));

  selector_playbutton->signal_clicked().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_filechooser_play));

  fsbutton->signal_file_set().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_filechooser_select));

  Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
  selection->signal_changed().connect(sigc::mem_fun(*this, &SoundPreferencePanel::on_sound_events_changed));

  Gtk::TreeModel::iterator iter = sound_store->children().begin();
  if (iter)
    {
      selection->select(iter);
    }

  update_senstives();
  set_border_width(12);
}

void
SoundPreferencePanel::on_sound_changed()
{
  int idx = sound_button->get_active_row_number();
  sound_theme->sound_enabled().set(idx > 0);
  update_senstives();
}

void
SoundPreferencePanel::update_senstives()
{
  int idx = sound_button->get_active_row_number();
  sound_treeview.set_sensitive(idx > 0);
  if (sound_theme_button != nullptr)
    {
      sound_theme_button->set_sensitive(idx > 0);
    }
  if (sound_volume_scale != nullptr)
    {
      sound_volume_scale->set_sensitive(idx > 0);
    }
  if (sound_play_button != nullptr)
    {
      sound_play_button->set_sensitive(idx > 0);
    }
  if (fsbutton != nullptr)
    {
      fsbutton->set_sensitive(idx > 0);
    }
}

void
SoundPreferencePanel::on_sound_enabled(const Glib::ustring &path_string)
{
  Gtk::TreePath path(path_string);
  const Gtk::TreeModel::iterator iter = sound_store->get_iter(path);

  if (iter)
    {
      Gtk::TreeRow row = *iter;
      std::string id = (Glib::ustring)row[sound_model.label];
      SoundEvent event = SoundTheme::sound_id_to_event(id);

      sound_theme->sound_event_enabled(event).set(row[sound_model.enabled]);
    }
}

void
SoundPreferencePanel::on_sound_play()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();

  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::string id = (Glib::ustring)row[sound_model.label];
      SoundEvent event = SoundTheme::sound_id_to_event(id);

      std::string filename = sound_theme->sound_event(event)();
      if (!filename.empty())
        {
          sound_theme->play_sound(filename);
        }
    }
}

void
SoundPreferencePanel::on_sound_filechooser_play()
{
  std::string filename = fsbutton->get_filename();

  sound_theme->play_sound(filename);
}

void
SoundPreferencePanel::on_sound_filechooser_select()
{
  TRACE_ENTRY();
  std::string filename = fsbutton->get_filename();

  TRACE_VAR(filename, fsbutton_filename, inhibit_events);

  if (inhibit_events == 0 && !filename.empty() && fsbutton_filename != filename)
    {
      TRACE_MSG("ok");

      Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
      Gtk::TreeModel::iterator iter = selection->get_selected();

      if (iter)
        {
          Gtk::TreeModel::Row row = *iter;

          TRACE_VAR(row[sound_model.label]);

          std::string id = (Glib::ustring)row[sound_model.label];
          SoundEvent event = SoundTheme::sound_id_to_event(id);
          sound_theme->sound_event(event).set(filename);

          TRACE_VAR(filename);
          update_sound_theme_selection();
        }

      fsbutton_filename = filename;
    }
}

void
SoundPreferencePanel::on_sound_events_changed()
{
  TRACE_ENTRY();
  Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();

  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::string id = (Glib::ustring)row[sound_model.label];
      SoundEvent event = SoundTheme::sound_id_to_event(id);
      std::string filename = sound_theme->sound_event(event)();

      TRACE_VAR(filename);

      if (!filename.empty())
        {
          inhibit_events++;
          fsbutton_filename = filename;
          fsbutton->set_filename(filename);
          inhibit_events--;
        }
    }
}

void
SoundPreferencePanel::on_sound_theme_changed()
{
  TRACE_ENTRY();
  int idx = sound_theme_button->get_active_row_number();

  SoundTheme::ThemeInfos themes = sound_theme->get_themes();

  if (idx >= 0 && idx < (int)themes.size())
    {
      std::string theme_id = themes[idx]->theme_id;

      sound_theme->activate_theme(theme_id);

      Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
      Gtk::TreeModel::iterator iter = selection->get_selected();

      if (iter)
        {
          Gtk::TreeModel::Row row = *iter;
          std::string id = (Glib::ustring)row[sound_model.label];
          SoundEvent event = SoundTheme::sound_id_to_event(id);
          std::string filename = sound_theme->sound_event(event)();

          if (!filename.empty())
            {
              TRACE_VAR(filename, row[sound_model.label]);
              inhibit_events++;
              fsbutton_filename = filename;
              fsbutton->set_filename(filename);
              inhibit_events--;
            }
        }
    }
}

void
SoundPreferencePanel::update_sound_theme_selection()
{
  TRACE_ENTRY();
  SoundTheme::ThemeInfo::Ptr active_theme = sound_theme->get_active_theme();

  sound_theme_button->remove_all();

  int active_index = -1;
  for (SoundTheme::ThemeInfo::Ptr theme: sound_theme->get_themes())
    {
      sound_theme_button->append(theme->description);

      if (theme == active_theme)
        {
          sound_theme_button->set_active(active_index);
        }
      active_index++;
    }
}
