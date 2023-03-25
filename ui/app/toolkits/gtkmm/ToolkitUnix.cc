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
#include "X11SystrayAppletWindow.hh"
#include "GnomeSession.hh"

#if defined(HAVE_INDICATOR)
#  include "IndicatorAppletMenu.hh"
#endif

#include "BreakWindow.hh"

ToolkitUnix::ToolkitUnix(int argc, char **argv)
  : Toolkit(argc, argv)
{
#if defined(PLATFORM_OS_UNIX)
   XInitThreads();
#endif
  gdk_init(nullptr, nullptr);

  locker = std::make_shared<UnixLocker>();
}

void
ToolkitUnix::init(std::shared_ptr<IApplicationContext> app)
{
#if defined(PLATFORM_OS_UNIX)
  if (GUIConfig::force_x11()())
    {
      g_setenv("GDK_BACKEND", "x11", TRUE);
    }
#endif

  Toolkit::init(app);

  Glib::VariantType String(Glib::VARIANT_TYPE_STRING);
  gapp->add_action_with_parameter("confirm-notification", String, [this](const Glib::VariantBase &value) {
    Glib::Variant<Glib::ustring> s = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(value);
    notify_confirm(s.get());
  });
}

IBreakWindow::Ptr
ToolkitUnix::create_break_window(int screen_index, workrave::BreakId break_id, BreakFlags break_flags)
{
  auto ret = Toolkit::create_break_window(screen_index, break_id, break_flags);
  // FIXME: remove hack
  if (auto break_window = std::dynamic_pointer_cast<BreakWindow>(ret); break_window)
    {
      auto gdk_window = break_window->get_window()->gobj();
      locker->set_window(gdk_window);
    }
  return ret;
}

std::shared_ptr<Locker>
ToolkitUnix::get_locker()
{
  return locker;
}

void
ToolkitUnix::show_notification(const std::string &id,
                               const std::string &title,
                               const std::string &balloon,
                               std::function<void()> func)
{
  notify_add_confirm_function(id, func);
  auto notification = Gio::Notification::create("Workrave");
  notification->set_body(balloon);
  notification->set_default_action_variant("app.confirm-notification", Glib::Variant<Glib::ustring>::create(id));
  auto icon = Gio::ThemedIcon::create("dialog-information");
  notification->set_icon(icon);
  gapp->send_notification(id, notification);
}
