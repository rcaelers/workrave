// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include "ToolkitUnix.hh"

#include <X11/Xlib.h>

#include "debug.hh"
#include "utils/Platform.hh"
#include "ui/GUIConfig.hh"
// #include "GnomeSession.hh"

#if defined(HAVE_INDICATOR)
#  include "IndicatorAppletMenu.hh"
#endif

using namespace workrave::utils;

ToolkitUnix::ToolkitUnix(int argc, char **argv)
  : Toolkit(argc, argv)
{
}

void
ToolkitUnix::preinit(std::shared_ptr<workrave::config::IConfigurator> config)
{
  TRACE_ENTRY();
  if (GUIConfig::force_x11()())
    {
      spdlog::info("Forcing X11 backend -> Not implemented");
    }

  XInitThreads();

  locker = std::make_shared<UnixLocker>();
}

void
ToolkitUnix::init(std::shared_ptr<IApplicationContext> app)
{
  TRACE_ENTRY();

#if defined(HAVE_WAYLAND)
  if (Platform::running_on_wayland())
    {
      auto wm = std::make_shared<WaylandWindowManager>();
      bool success = wm->init();
      if (success)
        {
          wayland_window_manager = wm;
        }
    }
#endif

  Toolkit::init(app);

  // Glib::VariantType String(Glib::VARIANT_TYPE_STRING);
  // gapp->add_action_with_parameter("confirm-notification", String, [this](const Glib::VariantBase &value) {
  //   Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(value);
  //   notify_confirm(s.get());
  // });
}

std::shared_ptr<Locker>
ToolkitUnix::get_locker()
{
  return locker;
}

auto
ToolkitUnix::get_desktop_image() -> QPixmap
{
  return {};
}

#if defined(HAVE_WAYLAND)
auto
ToolkitUnix::get_wayland_window_manager() -> std::shared_ptr<WaylandWindowManager>
{
  return wayland_window_manager;
}
#endif

void
ToolkitUnix::show_notification(const std::string &id,
                               const std::string &title,
                               const std::string &balloon,
                               std::function<void()> func)
{
  // notify_add_confirm_function(id, func);
  // auto notification = Gio::Notification::create("Workrave");
  // notification->set_body(balloon);
  // notification->set_default_action_variant("app.confirm-notification", Glib::Variant<Glib::ustring>::create(id));
  // auto icon = Gio::ThemedIcon::create("dialog-information");
  // notification->set_icon(icon);
  // gapp->send_notification(id, notification);
}
