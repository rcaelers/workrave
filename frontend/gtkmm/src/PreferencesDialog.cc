// Copyright (C) 2002 - 2020 Raymond Penners <raymond@dotsphinx.com>
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
#  include "config.h"
#endif

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <assert.h>

#include <gtkmm/notebook.h>
#include <gtkmm/stock.h>
#include <gtkmm/menu.h>
#include <gtkmm/combobox.h>
#include <gtkmm/cellrenderer.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/filechooserbutton.h>

#include "StringUtil.hh"
#include "Locale.hh"

#include "GtkUtil.hh"
#include "Hig.hh"
#include "PreferencesDialog.hh"
#include "TimeEntry.hh"
#include "TimerBoxPreferencePage.hh"
#include "TimerPreferencesPanel.hh"
#include "Util.hh"
#include "GUI.hh"
#include "GUIConfig.hh"
#include "DataConnector.hh"
#include "Menus.hh"

#include "CoreFactory.hh"
#include "CoreConfig.hh"
#include "IConfigurator.hh"

#ifdef HAVE_DISTRIBUTION
#  include "NetworkPreferencePage.hh"
#endif

#ifndef WR_CHECK_VERSION
#  define WR_CHECK_VERSION(comp, major, minor, micro)                                                      \
    (comp##_MAJOR_VERSION > (major) || (comp##_MAJOR_VERSION == (major) && comp##_MINOR_VERSION > (minor)) \
     || (comp##_MAJOR_VERSION == (major) && comp##_MINOR_VERSION == (minor) && comp##_MICRO_VERSION >= (micro)))
#endif

#define RUNKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"

using namespace std;

PreferencesDialog::PreferencesDialog()
  : HigDialog(_("Preferences"), false, false)
  , sound_button(NULL)
  , block_button(NULL)
  , sound_theme_button(NULL)
  , icon_theme_button(NULL)
  , connector(NULL)
  , sound_volume_scale(NULL)
  , sound_play_button(NULL)
  , fsbutton(NULL)
  , filefilter(NULL)
#ifdef PLATFORM_OS_WINDOWS
  , sensitivity_adjustment(3, 0, 100)
  , sensitivity_box(NULL)
#endif
{
  TRACE_ENTER("PreferencesDialog::PreferencesDialog");

  connector = new DataConnector();
  inhibit_events = 0;

  // Pages
  Gtk::Widget *timer_page = Gtk::manage(create_timer_page());
  Gtk::Notebook *gui_page = Gtk::manage(new Gtk::Notebook());

#if !defined(PLATFORM_OS_MACOS)
  Gtk::Widget *gui_general_page = Gtk::manage(create_gui_page());
  gui_page->append_page(*gui_general_page, _("General"));
#endif

#if 1
  Gtk::Widget *gui_sounds_page = Gtk::manage(create_sounds_page());
  gui_page->append_page(*gui_sounds_page, _("Sounds"));
#endif

  Gtk::Widget *gui_mainwindow_page = Gtk::manage(create_mainwindow_page());
  gui_page->append_page(*gui_mainwindow_page, _("Status Window"));

#if !defined(PLATFORM_OS_MACOS)
  Gtk::Widget *gui_applet_page = Gtk::manage(create_applet_page());
  gui_page->append_page(*gui_applet_page, _("Applet"));
#endif

#ifdef HAVE_DISTRIBUTION
  Gtk::Widget *network_page = Gtk::manage(create_network_page());
#endif

  // Notebook
  add_page(_("Timers"), "time.png", *timer_page);
  add_page(_("User interface"), "display.png", *gui_page);
#ifdef HAVE_DISTRIBUTION
  add_page(_("Network"), "network.png", *network_page);
#endif

  // Gtk::Widget *plugins_page = Gtk::manage( new PluginsPreferencePage() );
  // add_page( _("Plugins"), "workrave-icon-huge.png", *plugins_page );

  // Dialog
  get_vbox()->pack_start(notebook, true, true, 0);
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  show_all();

  TRACE_EXIT();
}

//! Destructor.
PreferencesDialog::~PreferencesDialog()
{
  TRACE_ENTER("PreferencesDialog::~PreferencesDialog");

#if defined(HAVE_LANGUAGE_SELECTION)
  const Gtk::TreeModel::iterator &iter = languages_combo.get_active();
  const Gtk::TreeModel::Row row = *iter;
  const Glib::ustring code = row[languages_columns.code];

  GUIConfig::set_locale(code);
#endif

  ICore *core = CoreFactory::get_core();
  core->remove_operation_mode_override("preferences");

  delete connector;
#ifndef HAVE_GTK3
  delete filefilter;
#endif
  TRACE_EXIT();
}

Gtk::Widget *
PreferencesDialog::create_gui_page()
{
  // Block types
  block_button = Gtk::manage(new Gtk::ComboBoxText());
#if GTKMM_CHECK_VERSION(2, 24, 0)
  block_button->append(_("No blocking"));
  block_button->append(_("Block input"));
  block_button->append(_("Block input and screen"));
#else
  block_button->append_text(_("No blocking"));
  block_button->append_text(_("Block input"));
  block_button->append_text(_("Block input and screen"));
#endif

  int block_idx;
  switch (GUIConfig::get_block_mode())
    {
    case GUIConfig::BLOCK_MODE_NONE:
      block_idx = 0;
      break;
    case GUIConfig::BLOCK_MODE_INPUT:
      block_idx = 1;
      break;
    default:
      block_idx = 2;
    }
  block_button->set_active(block_idx);
  block_button->signal_changed().connect(sigc::mem_fun(*this, &PreferencesDialog::on_block_changed));

  // Options
  HigCategoryPanel *panel = Gtk::manage(new HigCategoryPanel(_("Options")));

  panel->add_label(_("Block mode:"), *block_button);

#if defined(HAVE_LANGUAGE_SELECTION)
  string current_locale = GUIConfig::get_locale();

  languages_model = Gtk::ListStore::create(languages_columns);
  languages_combo.set_model(languages_model);

  std::vector<std::string> all_linguas;
  StringUtil::split(string(ALL_LINGUAS), ' ', all_linguas);
  all_linguas.push_back("en");

  Locale::LanguageMap languages_current_locale;
  Locale::LanguageMap languages_native_locale;

  Locale::get_all_languages_in_current_locale(languages_current_locale);
  Locale::get_all_languages_in_native_locale(languages_native_locale);

  Gtk::TreeModel::iterator iter = languages_model->append();
  Gtk::TreeModel::Row row = *iter;
  row[languages_columns.current] = _("System default");
  row[languages_columns.native] = "";
  row[languages_columns.code] = "";
  row[languages_columns.enabled] = true;

  Gtk::TreeModel::iterator selected = iter;

  for (vector<std::string>::iterator i = all_linguas.begin(); i != all_linguas.end(); i++)
    {
      string code = *i;

      iter = languages_model->append();
      row = *iter;
      row[languages_columns.code] = code;
      row[languages_columns.enabled] = true;

      if (current_locale == code)
        {
          selected = iter;
        }

      string txt = languages_current_locale[code].language_name;
      if (txt.empty())
        {
          txt = "Unrecognized language: (" + code + ")";
        }
      else if (languages_current_locale[code].country_name != "")
        {
          txt += " (" + languages_current_locale[code].country_name + ")";
        }
      row[languages_columns.current] = txt;

      if (languages_current_locale[code].language_name != languages_native_locale[code].language_name)
        {
          txt = languages_native_locale[code].language_name;
          if (languages_native_locale[code].country_name != "")
            {
              txt += " (" + languages_native_locale[code].country_name + ")";
            }

          Glib::RefPtr<Pango::Layout> pl = create_pango_layout(txt);
          if (pl->get_unknown_glyphs_count() > 0)
            {
              txt = _("(font not available)");
              row[languages_columns.enabled] = false;
            }

          row[languages_columns.native] = txt;
        }
    }

  languages_model->set_sort_column(languages_columns.current, Gtk::SORT_ASCENDING);
  languages_model->set_sort_func(languages_columns.current, sigc::mem_fun(*this, &PreferencesDialog::on_cell_data_compare));

  languages_combo.pack_start(current_cellrenderer, true);
  languages_combo.pack_start(native_cellrenderer, false);

  languages_combo.set_cell_data_func(native_cellrenderer, sigc::mem_fun(*this, &PreferencesDialog::on_native_cell_data));
  languages_combo.set_cell_data_func(current_cellrenderer, sigc::mem_fun(*this, &PreferencesDialog::on_current_cell_data));

  languages_combo.set_active(selected);

  panel->add_label(_("Language:"), languages_combo);
#endif

  bool show_autostart = false;

#if defined(PLATFORM_OS_WINDOWS)
  show_autostart = true;
#elif defined(PLATFORM_OS_UNIX)
  const char *desktop = g_getenv("XDG_CURRENT_DESKTOP");
  show_autostart = (g_strcmp0(desktop, "Unity") == 0);
#endif

  if (show_autostart)
    {
      Gtk::Label *autostart_lab = Gtk::manage(GtkUtil::create_label(_("Start Workrave on logon"), false));
      autostart_cb = Gtk::manage(new Gtk::CheckButton());
      autostart_cb->add(*autostart_lab);
      autostart_cb->signal_toggled().connect(sigc::mem_fun(*this, &PreferencesDialog::on_autostart_toggled));
      panel->add_widget(*autostart_cb);

      connector->connect(GUIConfig::CFG_KEY_AUTOSTART, dc::wrap(autostart_cb));

#if defined(PLATFORM_OS_WINDOWS)
      char value[MAX_PATH];
      bool rc = Util::registry_get_value(RUNKEY, "Workrave", value);
      autostart_cb->set_active(rc);
#endif
    }

  Gtk::Label *trayicon_lab = Gtk::manage(GtkUtil::create_label(_("Show system tray icon"), false));
  trayicon_cb = Gtk::manage(new Gtk::CheckButton());
  trayicon_cb->add(*trayicon_lab);
  connector->connect(GUIConfig::CFG_KEY_TRAYICON_ENABLED, dc::wrap(trayicon_cb));

  panel->add_widget(*trayicon_cb, false, false);

  auto *force_x11_lab = Gtk::manage(
    GtkUtil::create_label_with_tooltip(_("Force the use of X11 on Wayland (requires restart of Workrave)"),
                                       _("Workrave does not fully support Wayland natively. "
                                         "This option forces the use of X11 on Wayland. "
                                         "Changing this option requires a restart of Workrave.")));

  force_x11_cb = Gtk::manage(new Gtk::CheckButton());
  force_x11_cb->add(*force_x11_lab);
  connector->connect(GUIConfig::CFG_KEY_FORCE_X11, dc::wrap(force_x11_cb));

  panel->add_widget(*force_x11_cb, false, false);

  update_icon_theme_combo();
  if (icon_theme_button != NULL)
    {
      panel->add_label(_("Icon Theme:"), *icon_theme_button);
    }

  panel->set_border_width(12);
  return panel;
}

Gtk::Widget *
PreferencesDialog::create_sounds_page()
{
  Gtk::VBox *panel = Gtk::manage(new Gtk::VBox(false, 6));

  // Options
  HigCategoryPanel *hig = Gtk::manage(new HigCategoryPanel(_("Sound Options")));
  panel->pack_start(*hig, false, false, 0);

  // Sound types
  sound_button = Gtk::manage(new Gtk::ComboBoxText());
#if GTKMM_CHECK_VERSION(2, 24, 0)
  sound_button->append(_("No sounds"));
  sound_button->append(_("Play sounds using sound card"));
  sound_button->append(_("Play sounds using built-in speaker"));
#else
  sound_button->append_text(_("No sounds"));
  sound_button->append_text(_("Play sounds using sound card"));
  sound_button->append_text(_("Play sounds using built-in speaker"));
#endif
  int idx;
  if (!SoundPlayer::is_enabled())
    idx = 0;
  else
    {
      if (SoundPlayer::DEVICE_SPEAKER == SoundPlayer::get_device())
        idx = 2;
      else
        idx = 1;
    }
  sound_button->set_active(idx);
  sound_button->signal_changed().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_changed));

  IGUI *gui = GUI::get_instance();
  SoundPlayer *snd = gui->get_sound_player();

  if (snd->capability(SOUND_CAP_VOLUME))
    {
      // Volume
      sound_volume_scale = Gtk::manage(new Gtk::HScale(0.0, 100.0, 0.0));
      sound_volume_scale->set_increments(1.0, 5.0);
      connector->connect(SoundPlayer::CFG_KEY_SOUND_VOLUME, dc::wrap(sound_volume_scale->get_adjustment()));

      hig->add_label(_("Volume:"), *sound_volume_scale, true, true);
    }

  hig->add_label(_("Sound:"), *sound_button);

  if (snd->capability(SOUND_CAP_MUTE))
    {
      // Volume
      mute_cb = Gtk::manage(new Gtk::CheckButton(_("Mute sounds during rest break and daily limit")));

      connector->connect(SoundPlayer::CFG_KEY_SOUND_MUTE, dc::wrap(mute_cb));

      hig->add_widget(*mute_cb, true, true);
    }

  if (snd->capability(SOUND_CAP_EDIT))
    {
      // Sound themes
      hig = Gtk::manage(new HigCategoryPanel(_("Sound Events"), true));
      panel->pack_start(*hig, true, true, 0);

      sound_theme_button = Gtk::manage(new Gtk::ComboBoxText());

      update_sound_theme_selection();

      sound_theme_button->signal_changed().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_theme_changed));
      hig->add_label(_("Sound Theme:"), *sound_theme_button);

      sound_store = Gtk::ListStore::create(sound_model);
      sound_treeview.set_model(sound_store);

      for (unsigned int i = 0; i < SOUND_MAX; i++)
        {
          Gtk::TreeModel::iterator iter = sound_store->append();
          Gtk::TreeModel::Row row = *iter;

          bool sound_enabled = false;
          snd->get_sound_enabled((SoundEvent)i, sound_enabled);

          row[sound_model.enabled] = sound_enabled;
          row[sound_model.selectable] = true;
          row[sound_model.description] = _(SoundPlayer::sound_registry[i].friendly_name);
          row[sound_model.label] = SoundPlayer::sound_registry[i].id;
          row[sound_model.event] = i;
        }

      sound_treeview.set_rules_hint();
      sound_treeview.set_search_column(sound_model.description.index());

      int cols_count = sound_treeview.append_column_editable(_("Play"), sound_model.enabled);
      Gtk::TreeViewColumn *column = sound_treeview.get_column(cols_count - 1);

      Gtk::CellRendererToggle *cell =
        dynamic_cast<Gtk::CellRendererToggle *>(sound_treeview.get_column_cell_renderer(cols_count - 1));

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

#ifdef HAVE_GTK3
      filefilter = Gtk::FileFilter::create();
#else
      filefilter = new Gtk::FileFilter();
#endif

      filefilter->set_name(_("Wavefiles"));
#ifdef PLATFORM_OS_WINDOWS
      filefilter->add_pattern("*.wav");
#else
      filefilter->add_mime_type("audio/x-wav");
#endif
#ifdef HAVE_GTK3
      fsbutton->add_filter(filefilter);
#else
      fsbutton->add_filter(*filefilter);
#endif

      hig->add_widget(*hbox);

      Gtk::HBox *selector_hbox = Gtk::manage(new Gtk::HBox(false, 0));
      Gtk::Button *selector_playbutton = Gtk::manage(new Gtk::Button(_("Play")));

      selector_hbox->pack_end(*selector_playbutton, false, false, 0);
      selector_playbutton->show();
      fsbutton->set_extra_widget(*selector_hbox);

      cell->signal_toggled().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_enabled));

      sound_play_button->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_play));

      selector_playbutton->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_filechooser_play));

#if WR_CHECK_VERSION(GTKMM, 2, 22, 0)
      fsbutton->signal_file_set().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_filechooser_select));
#else
      fsbutton->signal_selection_changed().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_filechooser_select));
#endif

      Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
      selection->signal_changed().connect(sigc::mem_fun(*this, &PreferencesDialog::on_sound_events_changed));

      Gtk::TreeModel::iterator iter = sound_store->children().begin();
      if (iter)
        {
          selection->select(iter);
        }

      update_senstives();
    }

  panel->set_border_width(12);
  return panel;
}

Gtk::Widget *
PreferencesDialog::create_timer_page()
{
  // Timers page
  Gtk::Notebook *tnotebook = Gtk::manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);
  Glib::RefPtr<Gtk::SizeGroup> hsize_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  Glib::RefPtr<Gtk::SizeGroup> vsize_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_VERTICAL);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      // Label
      Gtk::Widget *box = Gtk::manage(GtkUtil::create_label_for_break((BreakId)i));
      TimerPreferencesPanel *tp = Gtk::manage(new TimerPreferencesPanel(BreakId(i), hsize_group, vsize_group));
      box->show_all();
#ifdef HAVE_GTK3
      tnotebook->append_page(*tp, *box);
#else
      tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*tp, *box));
#endif
    }

  Gtk::Widget *box = Gtk::manage(GtkUtil::create_label("Monitoring", false));
  Gtk::Widget *monitoring_page = create_monitoring_page();

#ifdef HAVE_GTK3
  tnotebook->append_page(*monitoring_page, *box);
#else
  tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*monitoring_page, *box));
#endif

  return tnotebook;
}

Gtk::Widget *
PreferencesDialog::create_monitoring_page()
{
  Gtk::VBox *panel = Gtk::manage(new Gtk::VBox(false, 6));
  panel->set_border_width(12);

#if defined(PLATFORM_OS_WINDOWS)
  Gtk::Label *monitor_type_lab = Gtk::manage(GtkUtil::create_label(_("Use alternate monitor"), false));
  monitor_type_cb = Gtk::manage(new Gtk::CheckButton());
  monitor_type_cb->add(*monitor_type_lab);
  monitor_type_cb->signal_toggled().connect(sigc::mem_fun(*this, &PreferencesDialog::on_monitor_type_toggled));
  panel->pack_start(*monitor_type_cb, false, false, 0);

  Gtk::Label *monitor_type_help1 =
    Gtk::manage(GtkUtil::create_label(_("Enable this option if Workrave fails to detect when you are using your computer"), false));
  panel->pack_start(*monitor_type_help1, false, false, 0);
  Gtk::Label *monitor_type_help2 =
    Gtk::manage(GtkUtil::create_label(_("Workrave needs to be restarted manually after changing this setting"), false));
  panel->pack_start(*monitor_type_help2, false, false, 0);

  sensitivity_box = Gtk::manage(new Gtk::HBox());
  Gtk::Widget *sensitivity_lab = Gtk::manage(GtkUtil::create_label_with_tooltip(
    _("Mouse sensitivity:"), _("Number of pixels the mouse should move before it is considered activity.")));
  Gtk::SpinButton *sensitivity_spin = Gtk::manage(new Gtk::SpinButton(sensitivity_adjustment));
  sensitivity_box->pack_start(*sensitivity_lab, false, false, 0);
  sensitivity_box->pack_start(*sensitivity_spin, false, false, 0);
  panel->pack_start(*sensitivity_box, false, false, 0);

  connector->connect(CoreConfig::CFG_KEY_MONITOR_SENSITIVITY, dc::wrap(&sensitivity_adjustment));

  string monitor_type;
  CoreFactory::get_configurator()->get_value_with_default("advanced/monitor", monitor_type, "default");

  monitor_type_cb->set_active(monitor_type != "default");

  sensitivity_box->set_sensitive(monitor_type != "default");
#endif

  debug_btn = Gtk::manage(new Gtk::Button(_("Debug monitoring")));
  debug_btn->signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDialog::on_debug_pressed));
  panel->pack_start(*debug_btn, false, false, 0);

  return panel;
}

Gtk::Widget *
PreferencesDialog::create_mainwindow_page()
{
  // Timers page
  return new TimerBoxPreferencePage("main_window");
}

Gtk::Widget *
PreferencesDialog::create_applet_page()
{
  // Timers page
  return new TimerBoxPreferencePage("applet");
}

#ifdef HAVE_DISTRIBUTION
Gtk::Widget *
PreferencesDialog::create_network_page()
{
  return new NetworkPreferencePage();
}
#endif

void
PreferencesDialog::add_page(const char *label, const char *image, Gtk::Widget &widget)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf(image);
  notebook.add_page(label, pixbuf, widget);
}

void
PreferencesDialog::on_sound_changed()
{
  int idx = sound_button->get_active_row_number();
  SoundPlayer::set_enabled(idx > 0);
  if (idx > 0)
    {
      SoundPlayer::Device dev = idx == 1 ? SoundPlayer::DEVICE_SOUNDCARD : SoundPlayer::DEVICE_SPEAKER;
      SoundPlayer::set_device(dev);
    }

  update_senstives();
}

void
PreferencesDialog::update_senstives()
{
  int idx = sound_button->get_active_row_number();
  if (idx > 0)
    {
      SoundPlayer::Device dev = idx == 1 ? SoundPlayer::DEVICE_SOUNDCARD : SoundPlayer::DEVICE_SPEAKER;

      sound_treeview.set_sensitive(dev == SoundPlayer::DEVICE_SOUNDCARD);
      if (sound_theme_button != NULL)
        {
          sound_theme_button->set_sensitive(dev == SoundPlayer::DEVICE_SOUNDCARD);
        }
      if (sound_volume_scale != NULL)
        {
          sound_volume_scale->set_sensitive(dev == SoundPlayer::DEVICE_SOUNDCARD);
        }
      if (sound_play_button != NULL)
        {
          sound_play_button->set_sensitive(dev == SoundPlayer::DEVICE_SOUNDCARD);
        }
      if (fsbutton != NULL)
        {
          fsbutton->set_sensitive(dev == SoundPlayer::DEVICE_SOUNDCARD);
        }
    }
}

void
PreferencesDialog::on_block_changed()
{
  int idx = block_button->get_active_row_number();
  GUIConfig::BlockMode m;
  switch (idx)
    {
    case 0:
      m = GUIConfig::BLOCK_MODE_NONE;
      break;
    case 1:
      m = GUIConfig::BLOCK_MODE_INPUT;
      break;
    default:
      m = GUIConfig::BLOCK_MODE_ALL;
    }
  GUIConfig::set_block_mode(m);
}

int
PreferencesDialog::run()
{
  show_all();
  return 0;
}

bool
PreferencesDialog::on_focus_in_event(GdkEventFocus *event)
{
  TRACE_ENTER("PreferencesDialog::focus_in");

  GUIConfig::BlockMode block_mode = GUIConfig::get_block_mode();
  if (block_mode != GUIConfig::BLOCK_MODE_NONE)
    {
      ICore *core = CoreFactory::get_core();

      OperationMode mode = core->get_operation_mode();
      if (mode == OPERATION_MODE_NORMAL)
        {
          core->set_operation_mode_override(OPERATION_MODE_QUIET, "preferences");
        }
    }
  TRACE_EXIT();
  return HigDialog::on_focus_in_event(event);
}

bool
PreferencesDialog::on_focus_out_event(GdkEventFocus *event)
{
  TRACE_ENTER("PreferencesDialog::focus_out");
  ICore *core = CoreFactory::get_core();

  core->remove_operation_mode_override("preferences");
  TRACE_EXIT();
  return HigDialog::on_focus_out_event(event);
}

#if defined(HAVE_LANGUAGE_SELECTION)
void
PreferencesDialog::on_current_cell_data(const Gtk::TreeModel::const_iterator &iter)
{
  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;
      Glib::ustring name = row[languages_columns.current];
      bool enabled = row[languages_columns.enabled];

      current_cellrenderer.set_property("text", name);
      current_cellrenderer.set_property("sensitive", enabled);
    }
}

void
PreferencesDialog::on_native_cell_data(const Gtk::TreeModel::const_iterator &iter)
{
  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;
      Glib::ustring name = row[languages_columns.native];
      bool enabled = row[languages_columns.enabled];

      native_cellrenderer.set_property("text", name);
      native_cellrenderer.set_property("sensitive", enabled);
    }
}

int
PreferencesDialog::on_cell_data_compare(const Gtk::TreeModel::iterator &iter1, const Gtk::TreeModel::iterator &iter2)
{
  Gtk::TreeModel::Row row1 = *iter1;
  Gtk::TreeModel::Row row2 = *iter2;
  Glib::ustring name1 = row1[languages_columns.current];
  Glib::ustring name2 = row2[languages_columns.current];
  Glib::ustring code1 = row1[languages_columns.code];
  Glib::ustring code2 = row2[languages_columns.code];

  if (code1 == "")
    {
      return -1;
    }
  else if (code2 == "")
    {
      return 1;
    }
  else
    {
      return g_utf8_collate(name1.c_str(), name2.c_str());
    }
}
#endif

void
PreferencesDialog::on_autostart_toggled()
{
#if defined(PLATFORM_OS_WINDOWS)
  bool on = autostart_cb->get_active();

  gchar *value = NULL;

  if (on)
    {
      string appdir = Util::get_application_directory();
      value = g_strdup_printf("%s" G_DIR_SEPARATOR_S "lib" G_DIR_SEPARATOR_S "workrave.exe", appdir.c_str());
    }

  Util::registry_set_value(RUNKEY, "Workrave", value);
#endif
}

#if defined(PLATFORM_OS_WINDOWS)
void
PreferencesDialog::on_monitor_type_toggled()
{
  bool on = monitor_type_cb->get_active();
  CoreFactory::get_configurator()->set_value("advanced/monitor", on ? "lowlevel" : "default");
  sensitivity_box->set_sensitive(on);
}
#endif

void
PreferencesDialog::on_sound_enabled(const Glib::ustring &path_string)
{
  Gtk::TreePath path(path_string);
  const Gtk::TreeModel::iterator iter = sound_store->get_iter(path);

  if (iter)
    {
      Gtk::TreeRow row = *iter;

      IGUI *gui = GUI::get_instance();
      SoundPlayer *snd = gui->get_sound_player();

      snd->set_sound_enabled((SoundEvent)(int)row[sound_model.event], row[sound_model.enabled]);
    }
}

void
PreferencesDialog::on_sound_play()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();

  if (iter)
    {
      IGUI *gui = GUI::get_instance();
      SoundPlayer *snd = gui->get_sound_player();
      Gtk::TreeModel::Row row = *iter;

      string filename;
      bool valid = snd->get_sound_wav_file((SoundEvent)(int)row[sound_model.event], filename);

      if (valid && filename != "")
        {
          snd->play_sound(filename);
        }
    }
}

void
PreferencesDialog::on_sound_filechooser_play()
{
  string filename = fsbutton->get_filename();

  IGUI *gui = GUI::get_instance();
  SoundPlayer *snd = gui->get_sound_player();
  snd->play_sound(filename);
}

void
PreferencesDialog::on_sound_filechooser_select()
{
  TRACE_ENTER("PreferencesDialog::on_sound_filechooser_select");
  string filename = fsbutton->get_filename();

  TRACE_MSG(filename << " " << fsbutton_filename << " " << inhibit_events);

  if (inhibit_events == 0 && filename != "" && fsbutton_filename != filename)
    {
      TRACE_MSG("ok");

      Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
      Gtk::TreeModel::iterator iter = selection->get_selected();

      if (iter)
        {
          Gtk::TreeModel::Row row = *iter;

          TRACE_MSG(row[sound_model.label]);

          IGUI *gui = GUI::get_instance();
          SoundPlayer *snd = gui->get_sound_player();
          snd->set_sound_wav_file((SoundEvent)(int)row[sound_model.event], filename);
          TRACE_MSG(filename);
          update_sound_theme_selection();
        }

      fsbutton_filename = filename;
      TRACE_EXIT();
    }
}

void
PreferencesDialog::on_sound_events_changed()
{
  TRACE_ENTER("PreferencesDialog::on_sound_events_changed");
  Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
  Gtk::TreeModel::iterator iter = selection->get_selected();

  if (iter)
    {
      Gtk::TreeModel::Row row = *iter;

      IGUI *gui = GUI::get_instance();
      SoundPlayer *snd = gui->get_sound_player();

      string filename;
      bool valid = snd->get_sound_wav_file((SoundEvent)(int)row[sound_model.event], filename);
      TRACE_MSG(filename);

      if (valid && filename != "")
        {
          inhibit_events++;
          fsbutton_filename = filename;
          fsbutton->set_filename(filename);
          inhibit_events--;
        }
    }
  TRACE_EXIT();
}

void
PreferencesDialog::on_sound_theme_changed()
{
  TRACE_ENTER("PreferencesDialog::on_sound_theme_changed");
  int idx = sound_theme_button->get_active_row_number();

  if (idx >= 0 && idx < (int)sound_themes.size())
    {
      SoundPlayer::Theme &theme = sound_themes[idx];

      IGUI *gui = GUI::get_instance();
      SoundPlayer *snd = gui->get_sound_player();
      snd->activate_theme(theme);

      Glib::RefPtr<Gtk::TreeSelection> selection = sound_treeview.get_selection();
      Gtk::TreeModel::iterator iter = selection->get_selected();

      if (iter)
        {
          Gtk::TreeModel::Row row = *iter;
          string event = (Glib::ustring)row[sound_model.label];
          string filename;
          bool valid = snd->get_sound_wav_file((SoundEvent)(int)row[sound_model.event], filename);

          if (valid && filename != "")
            {
              TRACE_MSG(filename << " " << row[sound_model.label]);
              inhibit_events++;
              fsbutton_filename = filename;
              fsbutton->set_filename(filename);
              inhibit_events--;
            }
        }
    }
  TRACE_EXIT();
}

void
PreferencesDialog::update_sound_theme_selection()
{
  TRACE_ENTER("PreferencesDialog::update_sound_theme_selection");
  sound_themes.erase(sound_themes.begin(), sound_themes.end());

  IGUI *gui = GUI::get_instance();
  SoundPlayer *snd = gui->get_sound_player();
  snd->get_sound_themes(sound_themes);

#if GTKMM_CHECK_VERSION(2, 24, 0)
  sound_theme_button->remove_all();
#else
  sound_theme_button->clear_items();
#endif

  int idx = 0;
  for (vector<SoundPlayer::Theme>::iterator it = sound_themes.begin(); it != sound_themes.end(); it++)
    {
#if GTKMM_CHECK_VERSION(2, 24, 0)
      sound_theme_button->append(it->description);
#else
      sound_theme_button->append_text(it->description);
#endif

      if (it->active)
        {
          sound_theme_button->set_active(idx);
        }
      idx++;
    }
  TRACE_EXIT();
}

void
PreferencesDialog::on_icon_theme_changed()
{
  TRACE_ENTER("PreferencesDialog::on_icon_theme_changed");
  int idx = icon_theme_button->get_active_row_number();

  if (idx == 0)
    {
      GUIConfig::set_icon_theme("");
    }
  else
    {
      GUIConfig::set_icon_theme(icon_theme_button->get_active_text());
    }
  TRACE_EXIT();
}

void
PreferencesDialog::update_icon_theme_combo()
{
  TRACE_ENTER("PreferencesDialog::update_icon_theme_combo");
  std::list<std::string> themes;

  for (const auto &dirname: Util::get_search_path(Util::SEARCH_PATH_IMAGES))
    {
      if (!g_str_has_suffix(dirname.c_str(), "images"))
        {
          continue;
        }

      GDir *dir = g_dir_open(dirname.c_str(), 0, NULL);
      if (dir != NULL)
        {
          const char *file;
          while ((file = g_dir_read_name(dir)) != NULL)
            {

              gchar *test_path = g_build_filename(dirname.c_str(), file, NULL);
              if (test_path != NULL && g_file_test(test_path, G_FILE_TEST_IS_DIR))
                {
                  themes.push_back(file);
                }
              g_free(test_path);
            }
          g_dir_close(dir);
        }
    }

  if (themes.size() > 0)
    {
      icon_theme_button = Gtk::manage(new Gtk::ComboBoxText());

#if GTKMM_CHECK_VERSION(2, 24, 0)
      icon_theme_button->append(_("Default"));
#else
      icon_theme_button->append_text(_("Default"));
#endif
      icon_theme_button->set_active(0);

      std::string current_icontheme = GUIConfig::get_icon_theme();
      int idx = 1;
      for (auto &theme: themes)
        {
#if GTKMM_CHECK_VERSION(2, 24, 0)
          icon_theme_button->append(theme);
#else
          icon_theme_button->append_text(theme);
#endif
          if (current_icontheme == theme)
            {
              icon_theme_button->set_active(idx);
            }
          idx++;
        }
      icon_theme_button->signal_changed().connect(sigc::mem_fun(*this, &PreferencesDialog::on_icon_theme_changed));
    }

  TRACE_EXIT();
}

void
PreferencesDialog::on_debug_pressed()
{
  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
  menus->on_menu_debug();
}
