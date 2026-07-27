// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#include "UnixLocker.hh"

#include <string>

#include <QByteArray>
#include <QGuiApplication>
#include <qguiapplication_platform.h>

#ifdef signals
#  undef signals
#endif
#include <gio/gio.h>
#include <spdlog/spdlog.h>
#include <xcb/xcb.h>
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "debug.hh"
#include "session/System.hh"
#include "utils/Platform.hh"

using namespace workrave::utils;

namespace
{
  constexpr unsigned int interesting_modifiers = ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask;
}

UnixLocker::UnixLocker()
{
  grab_retry_timer.setInterval(2000);
  QObject::connect(&grab_retry_timer, &QTimer::timeout, [this]() { on_lock_retry_timer(); });
}

bool
UnixLocker::can_lock()
{
  return !Platform::running_on_wayland();
}

void
UnixLocker::set_window(WId window)
{
  grab_window = window;
}

void
UnixLocker::prepare_lock()
{
}

void
UnixLocker::lock()
{
  spdlog::debug("UnixLocker::lock()");
  if (Platform::running_on_wayland())
    {
      return;
    }

  grab_wanted = true;
  if (!grabbed)
    {
      grabbed = lock_internal();
      spdlog::debug("UnixLocker::lock() wanted {} {}", grabbed, static_cast<unsigned long long>(grab_window));
      if (!grabbed && !grab_retry_timer.isActive())
        {
          grab_retry_timer.start();
        }
    }
}

bool
UnixLocker::lock_internal()
{
  TRACE_ENTRY();

  auto *display = static_cast<Display *>(x11_display());
  if (display == nullptr || grab_window == 0)
    {
      return false;
    }

  const auto window = static_cast<Window>(grab_window);
  const int keyboard_status = XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
  if (keyboard_status != GrabSuccess)
    {
      XFlush(display);
      return false;
    }

  const int pointer_status = XGrabPointer(display,
                                          window,
                                          True,
                                          ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                                          GrabModeAsync,
                                          GrabModeAsync,
                                          None,
                                          None,
                                          CurrentTime);
  if (pointer_status != GrabSuccess)
    {
      XUngrabKeyboard(display, CurrentTime);
      XFlush(display);
      return false;
    }

  query_desktop_lock_shortcuts();
  QGuiApplication::instance()->installNativeEventFilter(this);
  XFlush(display);
  return true;
}

void
UnixLocker::unlock()
{
  if (Platform::running_on_wayland())
    {
      return;
    }

  grab_wanted = false;
  grab_retry_timer.stop();

  auto *display = static_cast<Display *>(x11_display());
  if (display != nullptr)
    {
      XUngrabPointer(display, CurrentTime);
      XUngrabKeyboard(display, CurrentTime);
      XFlush(display);
      spdlog::debug("UnixLocker::unlock() ungrabbed");
    }

  if (grabbed)
    {
      QGuiApplication::instance()->removeNativeEventFilter(this);
    }
  grabbed = false;
}

auto
UnixLocker::nativeEventFilter(const QByteArray &event_type, void *message, qintptr *result) -> bool
{
  (void)result;

  if (event_type != "xcb_generic_event_t")
    {
      return false;
    }

  auto *event = static_cast<xcb_generic_event_t *>(message);
  const uint8_t response_type = event->response_type & ~0x80;
  if (response_type == XCB_KEY_PRESS)
    {
      auto *key_event = reinterpret_cast<xcb_key_press_event_t *>(event);
      spdlog::debug("Core KeyPress: keycode={} state=0x{:x} window=0x{:x}",
                    key_event->detail,
                    key_event->state,
                    key_event->event);
      handle_screen_lock_keystroke(key_event->detail, key_event->state);
    }
  else if (response_type == XCB_KEY_RELEASE)
    {
      auto *key_event = reinterpret_cast<xcb_key_release_event_t *>(event);
      spdlog::debug("Core KeyRelease: keycode={} state=0x{:x} window=0x{:x}",
                    key_event->detail,
                    key_event->state,
                    key_event->event);
    }

  return false;
}

void
UnixLocker::set_default_lock_shortcuts()
{
  spdlog::info("Using default screen lock shortcuts: Super+L and Ctrl+Alt+L");
  lock_shortcuts.emplace_back(Shortcut{XK_l, Mod4Mask, true});
  lock_shortcuts.emplace_back(Shortcut{XK_l, ControlMask | Mod1Mask, true});
}

void
UnixLocker::add_keybinding_shortcut(const char *binding, const char *schema_name, const char *key_name)
{
  if (binding == nullptr || binding[0] == '\0')
    {
      return;
    }

  KeySym keysym = 0;
  unsigned int modifiers = 0;
  if (parse_keybinding(binding, &keysym, &modifiers))
    {
      lock_shortcuts.emplace_back(Shortcut{keysym, modifiers, true});
      spdlog::info("Found screen lock shortcut from {}:{}: '{}'", schema_name, key_name, binding);
    }
}

void
UnixLocker::query_desktop_lock_shortcuts()
{
  lock_shortcuts.clear();

  struct SchemaInfo
  {
    const char *schema_name;
    const char *key_name;
  };

  const std::vector<SchemaInfo> schemas = {
    {.schema_name = "org.gnome.settings-daemon.plugins.media-keys", .key_name = "screensaver"},
    {.schema_name = "org.cinnamon.desktop.keybindings.media-keys", .key_name = "screensaver"},
    {.schema_name = "org.mate.SettingsDaemon.plugins.media-keys", .key_name = "screensaver"}};

  GSettingsSchemaSource *source = g_settings_schema_source_get_default();
  if (source == nullptr)
    {
      set_default_lock_shortcuts();
      return;
    }

  for (const auto &schema_info: schemas)
    {
      GSettingsSchema *schema = g_settings_schema_source_lookup(source, schema_info.schema_name, TRUE);
      if (schema == nullptr)
        {
          continue;
        }

      GSettings *settings = g_settings_new(schema_info.schema_name);
      if (settings == nullptr)
        {
          g_settings_schema_unref(schema);
          continue;
        }

      if (!g_settings_schema_has_key(schema, schema_info.key_name))
        {
          g_object_unref(settings);
          g_settings_schema_unref(schema);
          continue;
        }

      GVariant *variant = g_settings_get_value(settings, schema_info.key_name);
      if (variant == nullptr)
        {
          g_object_unref(settings);
          g_settings_schema_unref(schema);
          continue;
        }

      if (g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
        {
          const gchar *binding = g_variant_get_string(variant, nullptr);
          add_keybinding_shortcut(binding, schema_info.schema_name, schema_info.key_name);
        }
      else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING_ARRAY))
        {
          gsize length = 0;
          const gchar **bindings = g_variant_get_strv(variant, &length);
          if (bindings != nullptr)
            {
              for (gsize i = 0; i < length; i++)
                {
                  add_keybinding_shortcut(bindings[i], schema_info.schema_name, schema_info.key_name);
                }
              g_free(bindings);
            }
        }
      else
        {
          spdlog::warn("GSettings key '{}' has unexpected type: {}", schema_info.key_name, g_variant_get_type_string(variant));
        }

      g_variant_unref(variant);
      g_object_unref(settings);
      g_settings_schema_unref(schema);
    }

  if (lock_shortcuts.empty())
    {
      set_default_lock_shortcuts();
    }
}

bool
UnixLocker::parse_keybinding(const char *binding, KeySym *keysym, unsigned int *modifiers)
{
  if (binding == nullptr || binding[0] == '\0')
    {
      return false;
    }

  *modifiers = 0;
  *keysym = 0;

  std::string str(binding);
  std::string key;
  size_t pos = 0;

  while (pos < str.length())
    {
      if (str[pos] == '<')
        {
          size_t end = str.find('>', pos);
          if (end == std::string::npos)
            {
              return false;
            }

          std::string modifier = str.substr(pos + 1, end - pos - 1);
          if (modifier == "Primary" || modifier == "Control" || modifier == "Ctrl")
            {
              *modifiers |= ControlMask;
            }
          else if (modifier == "Shift")
            {
              *modifiers |= ShiftMask;
            }
          else if (modifier == "Alt" || modifier == "Mod1")
            {
              *modifiers |= Mod1Mask;
            }
          else if (modifier == "Super" || modifier == "Mod4")
            {
              *modifiers |= Mod4Mask;
            }
          else if (modifier == "Hyper" || modifier == "Mod3")
            {
              *modifiers |= Mod3Mask;
            }
          else if (modifier == "Meta" || modifier == "Mod2")
            {
              *modifiers |= Mod2Mask;
            }

          pos = end + 1;
        }
      else
        {
          key = str.substr(pos);
          break;
        }
    }

  if (key.empty())
    {
      return false;
    }

  *keysym = XStringToKeysym(key.c_str());
  return *keysym != NoSymbol;
}

void
UnixLocker::handle_screen_lock_keystroke(unsigned int keycode, unsigned int modifier_state)
{
  auto *display = static_cast<Display *>(x11_display());
  if (display == nullptr)
    {
      return;
    }

  const KeySym keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
  const unsigned int modifiers = modifier_state & interesting_modifiers;

  for (const auto &shortcut: lock_shortcuts)
    {
      if (shortcut.valid && shortcut.keysym == keysym && shortcut.modifiers == modifiers)
        {
          spdlog::info("Screen lock keystroke detected (keycode={}, keysym={}, mods=0x{:x}), locking screen",
                       keycode,
                       keysym,
                       modifiers);
          unlock();
          System::execute(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN);
          return;
        }
    }
}

void
UnixLocker::on_lock_retry_timer()
{
  if (grab_wanted)
    {
      lock();
    }

  if (!grab_wanted || grabbed)
    {
      grab_retry_timer.stop();
    }
}

auto
UnixLocker::x11_display() const -> void *
{
  auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
  if (native == nullptr)
    {
      return nullptr;
    }
  return native->display();
}
