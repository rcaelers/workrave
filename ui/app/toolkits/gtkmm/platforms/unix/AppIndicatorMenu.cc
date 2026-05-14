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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "AppIndicatorMenu.hh"

#include <filesystem>

#include <glib.h>

#include "utils/Signals.hh"
#include "ui/GUIConfig.hh"
#include "GtkUtil.hh"
#include "DbusMenu.hh"

AppIndicatorMenu::AppIndicatorMenu(std::shared_ptr<IPluginContext> context, std::shared_ptr<DbusMenu> dbus_menu)
  : context(context)
  , apphold(context->get_toolkit())
{
  indicator = app_indicator_new("workrave", "workrave", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

  GtkWidget *menu_widget = gtk_menu_new();
  app_indicator_set_menu(indicator, GTK_MENU(menu_widget));
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon(indicator, "workrave");

  g_signal_connect(G_OBJECT(indicator),
                   APP_INDICATOR_SIGNAL_CONNECTION_CHANGED,
                   G_CALLBACK(&AppIndicatorMenu::on_appindicator_connection_changed),
                   this);

  this->dbus_menu = dbus_menu;
  auto core = context->get_core();
  workrave::utils::connect(core->signal_operation_mode_changed(), tracker, [this](auto mode) {
    on_operation_mode_changed(mode);
  });
  auto menu_model = context->get_menu_model();
  workrave::utils::connect(menu_model->signal_update(), tracker, [this]() {
    update_dbus_menu_root();
  });
  workrave::OperationMode mode = core->get_regular_operation_mode();
  on_operation_mode_changed(mode);

  GUIConfig::trayicon_enabled().attach(tracker, [&](bool enabled) {
    app_indicator_set_status(indicator, enabled ? APP_INDICATOR_STATUS_ACTIVE : APP_INDICATOR_STATUS_PASSIVE);
  });
}

AppIndicatorMenu::~AppIndicatorMenu()
{
  // The timer belongs to the main context and could trigger, when this object is already destroyed.
  g_source_remove(apphold_release_timer_id);
}

void
AppIndicatorMenu::on_operation_mode_changed(workrave::OperationMode mode)
{
  std::string file;
  switch (mode)
    {
    case workrave::OperationMode::Normal:
      file = GtkUtil::get_image_filename("workrave-icon-medium.png");
      break;

    case workrave::OperationMode::Quiet:
      file = GtkUtil::get_image_filename("workrave-quiet-icon-medium.png");
      break;

    case workrave::OperationMode::Suspended:
      file = GtkUtil::get_image_filename("workrave-suspended-icon-medium.png");
      break;
    }

  if (!file.empty())
    {
      std::filesystem::path path(file);
      // app_indicator_set_icon shows an invalid icon when an extension is given
      path.replace_extension();
      std::string directory = path.parent_path().string();
      std::string filename = path.filename().string();
      app_indicator_set_icon_theme_path(indicator, directory.c_str());
      app_indicator_set_icon(indicator, filename.c_str());
    }

  update_dbus_menu_root();
}

void
AppIndicatorMenu::update_dbus_menu_root()
{
  DbusmenuServer *server{};
  g_object_get(indicator, "dbus-menu-server", &server, NULL);
  auto dbus_menu = this->dbus_menu.lock();
  auto *root_menu_item = dbus_menu->get_root_menu_item();
  g_object_ref(root_menu_item);
  dbusmenu_server_set_root(server, root_menu_item);
}

gboolean
AppIndicatorMenu::apphold_release(gpointer user_data)
{
  auto *self = static_cast<AppIndicatorMenu *>(user_data);
  if (!self->connected)
    {
      spdlog::info("AppIndicatorMenu: disconnected");
      self->apphold.release();
    }
  self->apphold_release_timer_id = 0;
  return G_SOURCE_REMOVE;
};

void
AppIndicatorMenu::on_appindicator_connection_changed(gpointer appindicator, gboolean connected, gpointer user_data)
{
  auto *self = static_cast<AppIndicatorMenu *>(user_data);
  if (connected)
    {
      self->connected = true;
      if (self->apphold_release_timer_id)
        {
          spdlog::info("AppIndicatorMenu: reconnected");
          g_source_remove(self->apphold_release_timer_id);
          self->apphold_release_timer_id = 0;
        }
      else
        {
          spdlog::info("AppIndicatorMenu: connected");
          self->apphold.hold();
        }
    }
  else
    {
      self->connected = false;
      guint waiting_time = 10;
      spdlog::info("AppIndicatorMenu: disconnected (waiting {} s for a reconnection)", waiting_time);
      // For filtering out cases, where system tray reloads only. It prevents the status window to show up in the meantime.
      self->apphold_release_timer_id = g_timeout_add_seconds(waiting_time, &AppIndicatorMenu::apphold_release, user_data);
    }
}
