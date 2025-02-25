// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "ToolkitWindows.hh"

#include <cstddef>
#include <gdk/gdk.h>
#include <gdk/gdkwin32.h>

#ifndef PLATFORM_OS_WINDOWS_NATIVE
#  include <pbt.h>
#endif
#include <wtsapi32.h>
#include <dbt.h>
#include <windows.h>

#include "ui/GUIConfig.hh"
#include "debug.hh"

using namespace workrave;
using namespace workrave::config;

ToolkitWindows::ToolkitWindows(int argc, char **argv)
  : Toolkit(argc, argv)
{
  gdk_init(nullptr, nullptr);
#if defined(HAVE_HARPOON)
  spdlog::info("Using Harpoon locker");
  locker = std::make_shared<WindowsHarpoonLocker>();
#else
  spdlog::info("Using standard locker");
  locker = std::make_shared<WindowsLocker>();
#endif
}

ToolkitWindows::~ToolkitWindows()
{
  TRACE_ENTRY();
}

void
ToolkitWindows::init(std::shared_ptr<IApplicationContext> app)
{
  // No auto hide scrollbars
  g_setenv("GTK_OVERLAY_SCROLLING", "0", TRUE);
  // No Windows-7 style client-side decorations on Windows 10...
  // TODO: check if still needed.
  // g_setenv("GTK_CSD", "0", TRUE);

  Toolkit::init(app);

  init_gui();
  theme_fixup();
  init_filter();
}

void
ToolkitWindows::deinit()
{
  main_window->get_window()->remove_filter(static_filter_func, this);

  Toolkit::deinit();
}

void
ToolkitWindows::release()
{
  Toolkit::release();

  if (!main_window->is_visible())
    {
      GUIConfig::trayicon_enabled().set(true);
    }
}

boost::signals2::signal<bool(MSG *msg), IToolkitWindows::event_combiner> &
ToolkitWindows::hook_event()
{
  return event_hook;
};

void
ToolkitWindows::init_gui()
{
  auto settings = Gtk::Settings::get_default();

  GUIConfig::light_dark_mode().attach(tracker, [settings](auto dark) {
    switch (dark)

      {
      case LightDarkTheme::Light:
        settings->property_gtk_application_prefer_dark_theme().set_value(false);
        break;
      case LightDarkTheme::Dark:
        settings->property_gtk_application_prefer_dark_theme().set_value(true);
        break;
      case LightDarkTheme::Auto:
        settings->property_gtk_application_prefer_dark_theme().set_value(is_windows_app_theme_dark());
        break;
      }
  });

  GUIConfig::theme_name().attach(tracker, [settings](auto name) {
    if (!name.empty())
      {
        settings->property_gtk_theme_name().set_value(name);
      }
  });

  settings->property_gtk_application_prefer_dark_theme().signal_changed().connect([settings]() {
    if (GUIConfig::light_dark_mode()() != LightDarkTheme::Auto)
      {
        GUIConfig::light_dark_mode().set(
          settings->property_gtk_application_prefer_dark_theme().get_value() ? LightDarkTheme::Dark : LightDarkTheme::Light);
      }
  });
  settings->property_gtk_theme_name().signal_changed().connect(
    [settings]() { GUIConfig::theme_name().set(settings->property_gtk_theme_name().get_value()); });
}

void
ToolkitWindows::theme_fixup()
{
  static const char css[] =
    R"(
       window decoration, tooltip decoration {
         all: unset;
       }
      )";
  auto provider = Gtk::CssProvider::create();
  provider->load_from_data(css);
  auto screen = Gdk::Screen::get_default();
  Gtk::StyleContext::add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_USER + 100);
}

#if !defined(GUID_DEVINTERFACE_MONITOR)
static GUID GUID_DEVINTERFACE_MONITOR = {0xe6f07b5f, 0xee97, 0x4a90, {0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7}};
#endif

void
ToolkitWindows::init_filter()
{
  main_window->get_window()->add_filter(static_filter_func, this);

  auto *window = (GtkWidget *)main_window->gobj();
  GdkWindow *gdk_window = gtk_widget_get_window(window);
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gdk_window);

  WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
  DEV_BROADCAST_DEVICEINTERFACE notification;
  ZeroMemory(&notification, sizeof(notification));
  notification.dbcc_size = sizeof(notification);
  notification.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  notification.dbcc_classguid = GUID_DEVINTERFACE_MONITOR;
  RegisterDeviceNotification(hwnd, &notification, DEVICE_NOTIFY_WINDOW_HANDLE);
}

GdkFilterReturn
ToolkitWindows::static_filter_func(void *xevent, GdkEvent *event, gpointer data)
{
  (void)event;
  auto *toolkit = static_cast<ToolkitWindows *>(data);
  auto result = toolkit->filter_func(static_cast<MSG *>(xevent));
  return result ? GDK_FILTER_CONTINUE : GDK_FILTER_REMOVE;
}

bool
ToolkitWindows::filter_func(MSG *msg)
{
  TRACE_ENTRY();
  switch (msg->message)
    {
    case WM_WTSSESSION_CHANGE:
      {
        if (msg->wParam == WTS_SESSION_LOCK)
          {
            signal_session_idle_changed()(true);
          }
        if (msg->wParam == WTS_SESSION_UNLOCK)
          {
            signal_session_idle_changed()(false);
            signal_session_unlocked()();
          }
      }
      break;

    case WM_POWERBROADCAST:
      {
        TRACE_MSG("WM_POWERBROADCAST {} {}", msg->wParam, msg->lParam);

        switch (msg->wParam)
          {
          case PBT_APMQUERYSUSPEND:
            TRACE_MSG("Query Suspend");
            break;

          case PBT_APMQUERYSUSPENDFAILED:
            TRACE_MSG("Query Suspend Failed");
            break;

          case PBT_APMRESUMESUSPEND:
          case PBT_APMRESUMEAUTOMATIC:
          case PBT_APMRESUMECRITICAL:
            {
              TRACE_MSG("Resume suspend");
              auto core = app->get_core();
              core->set_powersave(false);
            }
            break;

          case PBT_APMSUSPEND:
            {
              TRACE_MSG("Suspend");
              auto core = app->get_core();
              core->set_powersave(true);
            }
            break;
          }
      }
      break;

    case WM_DISPLAYCHANGE:
      {
        TRACE_MSG("WM_DISPLAYCHANGE {} {}", msg->wParam, msg->lParam);
      }
      break;

    case WM_TIMECHANGE:
      {
        TRACE_MSG("WM_TIMECHANGE {} {}", msg->wParam, msg->lParam);
        auto core = app->get_core();
        core->time_changed();
      }
      break;

    case WM_SETTINGCHANGE:
      {
        if (msg->lParam != 0 && _wcsicmp(L"ImmersiveColorSet", reinterpret_cast<wchar_t *>(msg->lParam)) == 0)
          {
            if (GUIConfig::light_dark_mode()() == LightDarkTheme::Auto)
              {
                logger->info("Theme change detected: switching to {} theme", is_windows_app_theme_dark() ? "dark" : "light");
                auto settings = Gtk::Settings::get_default();
                settings->property_gtk_application_prefer_dark_theme().set_value(is_windows_app_theme_dark());
              }
          }
      }

    case WM_DEVICECHANGE:
      {
        TRACE_MSG("WM_DEVICECHANGE {} {}", msg->wParam, msg->lParam);
        switch (msg->wParam)
          {
          case DBT_DEVICEARRIVAL:
          case DBT_DEVICEREMOVECOMPLETE:
            {
              logger->info("Display change detected");
              HWND hwnd = FindWindowExA(NULL, NULL, "GdkDisplayChange", NULL);
              if (hwnd)
                {
                  logger->info("Display change detected: sending WM_DISPLAYCHANGE");
                  SendMessage(hwnd, WM_DISPLAYCHANGE, 0, 0);
                }
            }
          default:
            break;
          }
        break;
      }
    }

  event_hook(msg);

  return true;
}

HWND
ToolkitWindows::get_event_hwnd() const
{
  return (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(main_window->Gtk::Widget::gobj()));
}

std::shared_ptr<Locker>
ToolkitWindows::get_locker()
{
  return locker;
}

bool
ToolkitWindows::is_windows_app_theme_dark()
{
  DWORD value = 1; // Default to light theme
  DWORD dataSize = sizeof(value);
  HKEY hKey = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey)
      == ERROR_SUCCESS)
    {
      RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &dataSize);
      RegCloseKey(hKey);
    }
  return value == 0;
}
