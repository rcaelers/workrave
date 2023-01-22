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

#include <memory>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "GeneralPreferencePanel.hh"

#include "commonui/nls.h"
#include "debug.hh"

#include "DataConnector.hh"

#include "GtkUtil.hh"
#include "Hig.hh"
#include "ui/GUIConfig.hh"
#include "utils/Platform.hh"
#include "utils/AssetPath.hh"
#include "utils/Paths.hh"

#include "commonui/Locale.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
#  define RUNKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#endif

using namespace workrave;
using namespace workrave::utils;

GeneralPreferencePanel::GeneralPreferencePanel(std::shared_ptr<IApplicationContext> app)
  : Gtk::VBox(false, 6)
  , app(app)
  , connector(std::make_shared<DataConnector>(app))

{
  TRACE_ENTRY();
  create_panel();
}

GeneralPreferencePanel::~GeneralPreferencePanel()
{
  TRACE_ENTRY();
#if defined(HAVE_LANGUAGE_SELECTION)
  const Gtk::TreeModel::iterator &iter = languages_combo.get_active();
  const Gtk::TreeModel::Row row = *iter;
  const Glib::ustring code = row[languages_columns.code];

  GUIConfig::locale().set(code);
#endif
}

void
GeneralPreferencePanel::create_panel()
{
  // Block types
  block_button = Gtk::manage(new Gtk::ComboBoxText());
  block_button->append(_("No blocking"));
  block_button->append(_("Block input"));
  block_button->append(_("Block input and screen"));

  int block_idx = 0;
  switch (GUIConfig::block_mode()())
    {
    case BlockMode::Off:
      block_idx = 0;
      break;
    case BlockMode::Input:
      block_idx = 1;
      break;
    default:
      block_idx = 2;
    }
  block_button->set_active(block_idx);
  block_button->signal_changed().connect(sigc::mem_fun(*this, &GeneralPreferencePanel::on_block_changed));

  // Options
  HigCategoryPanel *panel = Gtk::manage(new HigCategoryPanel(_("Options")));

  panel->add_label(_("Block mode:"), *block_button);

#if defined(HAVE_LANGUAGE_SELECTION)
  std::string current_locale = GUIConfig::locale()();

  languages_model = Gtk::ListStore::create(languages_columns);
  languages_combo.set_model(languages_model);

  std::vector<std::string> all_linguas;
  boost::split(all_linguas, ALL_LINGUAS, boost::is_any_of(" "));
  all_linguas.emplace_back("en");

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

  for (auto code: all_linguas)
    {
      iter = languages_model->append();
      row = *iter;
      row[languages_columns.code] = code;
      row[languages_columns.enabled] = true;

      if (current_locale == code)
        {
          selected = iter;
        }

      std::string txt = languages_current_locale[code].language_name;
      if (txt.empty())
        {
          txt = "Unrecognized language: (" + code + ")";
        }
      else if (!languages_current_locale[code].country_name.empty())
        {
          txt += " (" + languages_current_locale[code].country_name + ")";
        }
      row[languages_columns.current] = txt;

      if (languages_current_locale[code].language_name != languages_native_locale[code].language_name)
        {
          txt = languages_native_locale[code].language_name;
          if (!languages_native_locale[code].country_name.empty())
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
  languages_model->set_sort_func(languages_columns.current, sigc::mem_fun(*this, &GeneralPreferencePanel::on_cell_data_compare));

  languages_combo.pack_start(current_cellrenderer, true);
  languages_combo.pack_start(native_cellrenderer, false);

  languages_combo.set_cell_data_func(native_cellrenderer, sigc::mem_fun(*this, &GeneralPreferencePanel::on_native_cell_data));
  languages_combo.set_cell_data_func(current_cellrenderer, sigc::mem_fun(*this, &GeneralPreferencePanel::on_current_cell_data));

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
      autostart_cb->signal_toggled().connect(sigc::mem_fun(*this, &GeneralPreferencePanel::on_autostart_toggled));
      panel->add_widget(*autostart_cb);

      connector->connect(GUIConfig::autostart_enabled(), dc::wrap(autostart_cb));

#if defined(PLATFORM_OS_WINDOWS)
      char value[MAX_PATH];
      bool rc = Platform::registry_get_value(RUNKEY, "Workrave", value);
      autostart_cb->set_active(rc);
#endif
    }

  Gtk::Label *trayicon_lab = Gtk::manage(GtkUtil::create_label(_("Show system tray icon"), false));
  trayicon_cb = Gtk::manage(new Gtk::CheckButton());
  trayicon_cb->add(*trayicon_lab);
  connector->connect(GUIConfig::trayicon_enabled(), dc::wrap(trayicon_cb));

  panel->add_widget(*trayicon_cb, false, false);

#if defined(PLATFORM_OS_UNIX)
  auto *force_x11_lab = Gtk::manage(
    GtkUtil::create_label_with_tooltip(_("Force the use of X11 on Wayland (requires restart of Workrave)"),
                                       _("Workrave does not fully support Wayland natively. "
                                         "This option forces the use of X11 on Wayland. "
                                         "Changing this option requires a restart of Workrave.")));

  force_x11_cb = Gtk::manage(new Gtk::CheckButton());
  force_x11_cb->add(*force_x11_lab);
  connector->connect(GUIConfig::force_x11(), dc::wrap(force_x11_cb));

  panel->add_widget(*force_x11_cb, false, false);
#endif

  update_icon_theme_combo();
  if (icon_theme_button != nullptr)
    {
      panel->add_label(_("Icon Theme:"), *icon_theme_button);
    }

#if defined(PLATFORM_OS_WINDOWS)
  Gtk::Label *dark_lab = Gtk::manage(GtkUtil::create_label(_("Use dark theme"), false));
  dark_cb = Gtk::manage(new Gtk::CheckButton());
  dark_cb->add(*dark_lab);
  panel->add_widget(*dark_cb, false, false);

  connector->connect(GUIConfig::theme_dark(), dc::wrap(dark_cb));
#endif
  pack_start(*panel, false, false, 0);

  panel->set_border_width(12);
}

void
GeneralPreferencePanel::on_block_changed()
{
  int idx = block_button->get_active_row_number();
  BlockMode m;
  switch (idx)
    {
    case 0:
      m = BlockMode::Off;
      break;
    case 1:
      m = BlockMode::Input;
      break;
    default:
      m = BlockMode::All;
    }
  GUIConfig::block_mode().set(m);
}

#if defined(HAVE_LANGUAGE_SELECTION)
void
GeneralPreferencePanel::on_current_cell_data(const Gtk::TreeModel::const_iterator &iter)
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
GeneralPreferencePanel::on_native_cell_data(const Gtk::TreeModel::const_iterator &iter)
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
GeneralPreferencePanel::on_cell_data_compare(const Gtk::TreeModel::iterator &iter1, const Gtk::TreeModel::iterator &iter2)
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
GeneralPreferencePanel::on_autostart_toggled()
{
#if defined(PLATFORM_OS_WINDOWS)
  bool on = autostart_cb->get_active();

  gchar *value = nullptr;

  if (on)
    {
      auto exe = Paths::get_application_directory() / "bin" / "workrave.exe";

      Platform::registry_set_value(RUNKEY, "Workrave", exe.string().c_str());
    }
  else
    {
      Platform::registry_set_value(RUNKEY, "Workrave", nullptr);
    }
#endif
}

void
GeneralPreferencePanel::on_icon_theme_changed()
{
  TRACE_ENTRY();
  int idx = icon_theme_button->get_active_row_number();

  if (idx == 0)
    {
      GUIConfig::icon_theme().set("");
    }
  else
    {
      GUIConfig::icon_theme().set(icon_theme_button->get_active_text());
    }
}

void
GeneralPreferencePanel::update_icon_theme_combo()
{
  TRACE_ENTRY();
  std::list<std::string> themes;

  for (const auto &dirname: AssetPath::get_search_path(AssetPath::SEARCH_PATH_IMAGES))
    {
      if (!g_str_has_suffix(dirname.string().c_str(), "images"))
        {
          continue;
        }

      GDir *dir = g_dir_open(dirname.string().c_str(), 0, nullptr);
      if (dir != nullptr)
        {
          const char *file;
          while ((file = g_dir_read_name(dir)) != nullptr)
            {
              gchar *test_path = g_build_filename(dirname.string().c_str(), file, nullptr);
              if (test_path != nullptr && g_file_test(test_path, G_FILE_TEST_IS_DIR))
                {
                  themes.emplace_back(file);
                }
              g_free(test_path);
            }
          g_dir_close(dir);
        }
    }

  if (!themes.empty())
    {
      icon_theme_button = Gtk::manage(new Gtk::ComboBoxText());

      icon_theme_button->append(_("Default"));
      icon_theme_button->set_active(0);

      std::string current_icontheme = GUIConfig::icon_theme()();
      int idx = 1;
      for (auto &theme: themes)
        {
          icon_theme_button->append(theme);
          if (current_icontheme == theme)
            {
              icon_theme_button->set_active(idx);
            }
          idx++;
        }
      icon_theme_button->signal_changed().connect(sigc::mem_fun(*this, &GeneralPreferencePanel::on_icon_theme_changed));
    }
}
