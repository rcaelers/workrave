// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
// Copyright (C) 2025 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef APPINDICATORMENU_HH
#define APPINDICATORMENU_HH

#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "DbusMenu.hh"
#include "utils/Signals.hh"

#include "ui/Plugin.hh"
#include "ui/IPluginContext.hh"
#include "ToolkitMenu.hh"
#include "ui/AppHold.hh"

#if defined(HAVE_APPINDICATOR_AYATANA)
#  include <libayatana-appindicator/app-indicator.h>
#else
#  include <libappindicator/app-indicator.h>
#  endif

#include <gtk/gtk.h>

class AppIndicatorMenu
  : public sigc::trackable
  , public Plugin<AppIndicatorMenu, DbusMenu>
{
public:
  explicit AppIndicatorMenu(std::shared_ptr<IPluginContext> context, std::shared_ptr<DbusMenu> dbus_menu);
  ~AppIndicatorMenu() override;

  std::string get_plugin_id() const override
  {
    return "workrave.AppIndicatorMenu";
  }

private:
  void on_operation_mode_changed(workrave::OperationMode m);
  void update_dbus_menu_root();
  static void on_appindicator_connection_changed(gpointer appindicator, gboolean connected, gpointer user_data);
  static gboolean apphold_release(gpointer user_data);

private:
  std::shared_ptr<IPluginContext> context;
  AppHold apphold;
  bool connected;
  guint apphold_release_timer_id{0};
  std::weak_ptr<DbusMenu> dbus_menu;
  std::shared_ptr<ToolkitMenu> menu;
  AppIndicator *indicator{};

  workrave::utils::Trackable tracker;

private:
};

#endif // APPINDICATORMENU_HH
