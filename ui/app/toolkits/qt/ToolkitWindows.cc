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
#ifndef PLATFORM_OS_WINDOWS_NATIVE
#  include <pbt.h>
#endif
#include <wtsapi32.h>
#include <dbt.h>
#include <windows.h>

#include <QEvent>
#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>

#include "debug.hh"
#include "ui/GUIConfig.hh"

using namespace workrave;
using namespace workrave::config;

// Re-asserts the HWND's layered/opaque state to match the widget's
// Qt::WA_TranslucentBackground attribute. Needed because a window that was
// ever created translucent (frameless Sanctuary view) can keep compositing
// with a stale alpha surface even after WA_TranslucentBackground is cleared
// and the window is destroyed/recreated for the Classic view — forcing it
// here every time the native window is (re)created is more reliable than
// depending on Qt's own bookkeeping surviving that transition.
static void
reassert_native_window_opacity(HWND hwnd, bool translucent)
{
  if (hwnd == nullptr || !IsWindow(hwnd))
    {
      return;
    }

  LONG_PTR ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
  LONG_PTR new_ex_style = translucent ? (ex_style | WS_EX_LAYERED) : (ex_style & ~WS_EX_LAYERED);
  if (new_ex_style != ex_style)
    {
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, new_ex_style);
    }

  // Force the compositor to fully re-evaluate the window's frame/surface
  // instead of reusing a stale (alpha-enabled) composition surface.
  SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME | RDW_UPDATENOW);
}

ToolkitWindows::ToolkitWindows(int argc, char **argv)
  : Toolkit(argc, argv)
{
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

  Toolkit::init(app);

  init_gui();
  init_filter();
}

void
ToolkitWindows::deinit()
{
  Toolkit::deinit();
}

void
ToolkitWindows::release()
{
  Toolkit::release();

#if 0
if (!main_window->is_visible())
    {
      GUIConfig::trayicon_enabled().set(true);
    }
#endif
}

boost::signals2::signal<bool(MSG *msg), IToolkitWindows::event_combiner> &
ToolkitWindows::hook_event()
{
  return event_hook;
};

bool
ToolkitWindows::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == main_window && event->type() == QEvent::WinIdChange)
    {
      auto hwnd = reinterpret_cast<HWND>(main_window->winId());
      reassert_native_window_opacity(hwnd, main_window->testAttribute(Qt::WA_TranslucentBackground));
    }
  return Toolkit::eventFilter(obj, event);
}

void
ToolkitWindows::init_gui()
{
}

void
ToolkitWindows::apply_light_dark_mode(LightDarkTheme mode)
{
  Qt::ColorScheme scheme = Qt::ColorScheme::Unknown;
  switch (mode)
    {
    case LightDarkTheme::Light:
      scheme = Qt::ColorScheme::Light;
      break;
    case LightDarkTheme::Dark:
      scheme = Qt::ColorScheme::Dark;
      break;
    case LightDarkTheme::Auto:
      scheme = is_windows_app_theme_dark() ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light;
      break;
    }

  QGuiApplication::styleHints()->setColorScheme(scheme);
}

void
ToolkitWindows::init_filter()
{
  QCoreApplication::instance()->installNativeEventFilter(this);
}

bool
ToolkitWindows::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
  TRACE_ENTRY();
  (void)eventType;
  auto *msg = static_cast<MSG *>(message);
  switch (msg->message)
    {
    case WM_QUERYENDSESSION:
      {
        logger->info("Windows session end requested: reason=0x{:x}", static_cast<unsigned int>(msg->lParam));
        *result = TRUE;
        return true;
      }

    case WM_ENDSESSION:
      {
        logger->info("Windows session ending: end={}, reason=0x{:x}", msg->wParam != FALSE, static_cast<unsigned int>(msg->lParam));
        if (msg->wParam != FALSE)
          {
            request_graceful_shutdown((msg->lParam & ENDSESSION_CLOSEAPP) != 0 ? "Restart Manager close request"
                                                                               : "Windows session end");
          }
      }
      break;

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

          default:
            break;
          }
      }
      break;

    case WM_DISPLAYCHANGE:
      {
        TRACE_MSG("WM_DISPLAYCHANGE {} {}", msg->wParam, msg->lParam);
      }
      break;

#ifndef HAVE_CORE_NEXT
    case WM_TIMECHANGE:
      {
        TRACE_MSG("WM_TIMECHANGE {} {}", msg->wParam, msg->lParam);
        auto core = app->get_core();
        core->time_changed();
      }
      break;
#endif

    case WM_SETTINGCHANGE:
      {
        if (msg->lParam != 0 && _wcsicmp(L"ImmersiveColorSet", reinterpret_cast<wchar_t *>(msg->lParam)) == 0)
          {
            if (GUIConfig::light_dark_mode()() == LightDarkTheme::Auto)
              {
                logger->info("Theme change detected: switching to {} theme", is_windows_app_theme_dark() ? "dark" : "light");
                apply_light_dark_mode(LightDarkTheme::Auto);
              }
          }
      }
      break;

    default:
      break;
    }

  event_hook(msg);

  return false;
}

void
ToolkitWindows::request_graceful_shutdown(const char *reason)
{
  if (shutdown_requested)
    {
      return;
    }

  shutdown_requested = true;
  logger->info("Gracefully shutting down Workrave: {}", reason);

  get_locker()->unlock();

  QTimer::singleShot(0, this, [this]() { terminate(); });
}

HWND
ToolkitWindows::get_event_hwnd() const
{
  return 0;
}

auto
ToolkitWindows::get_desktop_image() -> QPixmap
{
  QPixmap pixmap;
  return pixmap;
}

std::shared_ptr<Locker>
ToolkitWindows::get_locker()
{
  return locker;
}

// TODO: Duplicate code gtkmm and qt toolkits. Move to platform.
bool
ToolkitWindows::is_windows_app_theme_dark()
{
  DWORD value = 1; // Default to light theme
  DWORD dataSize = sizeof(value);
  HKEY hKey = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey)
      == ERROR_SUCCESS)
    {
      RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &dataSize);
      RegCloseKey(hKey);
    }
  return value == 0;
}
