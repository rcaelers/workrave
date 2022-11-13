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

#ifndef GENERALPREFERENCEPANEL_HH
#define GENERALPREFERENCEPANEL_HH

#include <string>

#include "utils/Signals.hh"
#include "ui/IApplicationContext.hh"

#include "ui/prefwidgets/gtkmm/Widget.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"

#include <gtkmm.h>

class DataConnector;
class Configurator;

class GeneralPreferencePanel
  : public Gtk::VBox
  , public workrave::utils::Trackable
{
public:
  explicit GeneralPreferencePanel(std::shared_ptr<IApplicationContext> app);
  ~GeneralPreferencePanel() override;

private:
  void create_panel();

  void on_autostart_toggled();
  void on_block_changed();
  void on_icon_theme_changed();
  void update_icon_theme_combo();

private:
  std::shared_ptr<IApplicationContext> app;
  std::shared_ptr<DataConnector> connector;

#if defined(HAVE_LANGUAGE_SELECTION)
  // Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    {
      add(current);
      add(native);
      add(enabled);
      add(code);
    }

    Gtk::TreeModelColumn<bool> enabled;
    Gtk::TreeModelColumn<Glib::ustring> code;
    Gtk::TreeModelColumn<Glib::ustring> native;
    Gtk::TreeModelColumn<Glib::ustring> current;
  };

  void on_native_cell_data(const Gtk::TreeModel::const_iterator &iter);
  void on_current_cell_data(const Gtk::TreeModel::const_iterator &iter);
  int on_cell_data_compare(const Gtk::TreeModel::iterator &iter1, const Gtk::TreeModel::iterator &iter2);

  Gtk::ComboBox languages_combo;
  ModelColumns languages_columns;
  Glib::RefPtr<Gtk::ListStore> languages_model;
  Gtk::CellRendererText native_cellrenderer;
  Gtk::CellRendererText current_cellrenderer;
#endif

#if defined(PLATFORM_OS_UNIX)
  Gtk::CheckButton *force_x11_cb{nullptr};
#endif

#if defined(PLATFORM_OS_WINDOWS)
  Gtk::CheckButton *dark_cb{nullptr};
  Glib::RefPtr<Glib::Binding> dark_binding;
#endif

  std::shared_ptr<ui::prefwidgets::gtkmm::BoxWidget> general_frame;

  Gtk::ComboBoxText *block_button{nullptr};
  Gtk::CheckButton *autostart_cb{nullptr};
  Gtk::CheckButton *trayicon_cb{nullptr};
  Gtk::ComboBoxText *icon_theme_button{nullptr};
};

#endif // GENERALPREFERENCEPANEL_HH
