// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
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

#include <vector>

#include <gtkmm.h>

#include "Hig.hh"
#include "IconListNotebook.hh"

#include "core/ICore.hh"
#include "ui/IApplicationContext.hh"
#include "ui/SoundTheme.hh"
#include "ui/prefwidgets/gtkmm/Widget.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"

class TimeEntry;
class DataConnector;

namespace Gtk
{
  class ComboBox;
  class ComboBox;
  class FileChooserButton;
  class HScale;
  class FileFilter;
} // namespace Gtk

class PreferencesDialog : public HigDialog
{
public:
  explicit PreferencesDialog(std::shared_ptr<IApplicationContext> app);
  ~PreferencesDialog() override;

  int run();

private:
  void add_page(const char *label, const char *image, Gtk::Widget &widget);
  Gtk::Widget *create_gui_page();
  Gtk::Widget *create_timer_page();
  Gtk::Widget *create_sounds_page();
  Gtk::Widget *create_applet_page();
  Gtk::Widget *create_mainwindow_page();
  bool on_focus_in_event(GdkEventFocus *event) override;
  bool on_focus_out_event(GdkEventFocus *event) override;

  void on_sound_changed();
  void on_block_changed();

  Gtk::ComboBoxText *sound_button{nullptr};
  Gtk::ComboBoxText *block_button{nullptr};
  Gtk::ComboBoxText *sound_theme_button{nullptr};
  Gtk::ComboBoxText *icon_theme_button{nullptr};

  IconListNotebook notebook;

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

  std::shared_ptr<IApplicationContext> app;
  SoundTheme::Ptr sound_theme;
  DataConnector *connector;
  Gtk::TreeView sound_treeview;
  SoundModel sound_model;
  Glib::RefPtr<Gtk::ListStore> sound_store;
  Gtk::CellRendererToggle sound_enabled_cellrenderer;
  Gtk::CellRendererText sound_event_cellrenderer;
  Gtk::HScale *sound_volume_scale{nullptr};
  Gtk::Button *sound_play_button{nullptr};
  int inhibit_events;
  Gtk::CheckButton *mute_cb{nullptr};

  Gtk::FileChooserButton *fsbutton{nullptr};
  Glib::RefPtr<Gtk::FileFilter> filefilter;
  std::string fsbutton_filename;
  Gtk::CheckButton *trayicon_cb{nullptr};
  Gtk::CheckButton *force_x11_cb{nullptr};
  std::shared_ptr<ui::prefwidgets::gtkmm::BoxWidget> general_frame;
#if defined(PLATFORM_OS_WINDOWS)
  Gtk::CheckButton *dark_cb{nullptr};
  Glib::RefPtr<Glib::Binding> dark_binding;
#endif

  void on_sound_enabled(const Glib::ustring &path_stringxo);
  void on_sound_play();
  void on_sound_filechooser_play();
  void on_sound_filechooser_select();
  void on_sound_events_changed();
  void on_sound_theme_changed();
  void update_sound_theme_selection();
  void update_senstives();
  void on_icon_theme_changed();
  void update_icon_theme_combo();

  Gtk::CheckButton *autostart_cb{nullptr};
  void on_autostart_toggled();

  Gtk::Button *debug_btn{nullptr};
  void on_debug_pressed();

  Gtk::Widget *create_monitoring_page();

#if defined(PLATFORM_OS_WINDOWS)
  Gtk::CheckButton *monitor_type_cb{nullptr};
  void on_monitor_type_toggled();

  Glib::RefPtr<Gtk::Adjustment> sensitivity_adjustment{Gtk::Adjustment::create(3, 0, 100)};
  Gtk::HBox *sensitivity_box{nullptr};
#endif
};

#endif // PREFERENCESWINDOW_HH
