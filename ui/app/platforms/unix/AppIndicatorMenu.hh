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

#include <memory>
#include <string>

#include "utils/Signals.hh"

#include "ui/Plugin.hh"
#include "ui/IPluginContext.hh"
#include "ui/AppHold.hh"

#if defined(HAVE_APPINDICATOR_GLIB)
#  include <ayatana-appindicator.h>
#  include "GioMenu.hh"
#else
#  include <libayatana-appindicator/app-indicator.h>
#  include "DbusMenu.hh"
#endif

#if defined(HAVE_APPINDICATOR_GLIB)
class AppIndicatorMenu : public Plugin<AppIndicatorMenu>
#else
class AppIndicatorMenu : public Plugin<AppIndicatorMenu, DbusMenu>
#endif
{
public:
#if defined(HAVE_APPINDICATOR_GLIB)
  explicit AppIndicatorMenu(std::shared_ptr<IPluginContext> context);
#else
  explicit AppIndicatorMenu(std::shared_ptr<IPluginContext> context, std::shared_ptr<DbusMenu> dbus_menu);
#endif
  ~AppIndicatorMenu() override;

  std::string get_plugin_id() const override
  {
    return "workrave.AppIndicatorMenu";
  }

private:
  void on_operation_mode_changed(workrave::OperationMode m);
  static void on_appindicator_connection_changed(gpointer appindicator, gboolean connected, gpointer user_data);
  static gboolean apphold_release(gpointer user_data);

private:
  std::shared_ptr<IPluginContext> context;
  AppHold apphold;
  bool connected{false};
  guint apphold_release_timer_id{0};
#if defined(HAVE_APPINDICATOR_GLIB)
  std::unique_ptr<GioMenu> menu;
#endif
  AppIndicator *indicator{};

  workrave::utils::Trackable tracker;
};

#endif // APPINDICATORMENU_HH
