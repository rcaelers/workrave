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

#include <cstdio>
#include <vector>
#include <string>

#include <glibmm.h>
#include <gio/gio.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h>
#include <X11/X.h>

#include <spdlog/spdlog.h>

#include "debug.hh"
#include "utils/Platform.hh"
#include "session/System.hh"

using namespace workrave::utils;

bool
UnixLocker::can_lock()
{
  return !Platform::running_on_wayland();
}

void
UnixLocker::set_window(GdkWindow *window)
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
  if (!Platform::running_on_wayland())
    {
      grab_wanted = true;
      spdlog::debug("UnixLocker::lock() wanted");
      if (!grabbed)
        {
          grabbed = lock_internal();
          spdlog::debug("UnixLocker::lock() wanted {} {}", grabbed, (intptr_t)grab_window);
          if (!grabbed && !grab_retry_connection.connected())
            {
              grab_retry_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &UnixLocker::on_lock_retry_timer),
                                                                     2000);
            }
        }
    }
}

bool
UnixLocker::lock_internal()
{
  TRACE_ENTRY();
  bool ret = false;

#if GTK_CHECK_VERSION(3, 24, 0)
  // gdk_device_grab is deprecated since 3.20.
  // However, an issue that was solved in gtk 3.24 causes windows to be hidden
  // when a grab fails: https://github.com/GNOME/gtk/commit/2c8b95a518bea2192145efe11219f2e36091b37a
  GdkGrabStatus status = GDK_GRAB_FAILED;

  GdkDisplay *display = gdk_display_get_default();
  GdkSeat *seat = gdk_display_get_default_seat(display);
  status = gdk_seat_grab(seat, grab_window, GDK_SEAT_CAPABILITY_ALL, TRUE, nullptr, nullptr, nullptr, nullptr);

  ret = status == GDK_GRAB_SUCCESS;
#else
  GdkDevice *device = gtk_get_current_event_device();
  if (device == nullptr)
    {
      GdkDisplay *display = gdk_window_get_display(grab_window);
      GdkDeviceManager *device_manager = gdk_display_get_device_manager(display);
      device = gdk_device_manager_get_client_pointer(device_manager);
    }

  if (device != nullptr)
    {
      if (gdk_device_get_source(device) == GDK_SOURCE_KEYBOARD)
        {
          keyboard = device;
          pointer = gdk_device_get_associated_device(device);
        }
      else
        {
          pointer = device;
          keyboard = gdk_device_get_associated_device(device);
        }
    }

  GdkGrabStatus keybGrabStatus;
  keybGrabStatus = gdk_device_grab(keyboard,
                                   grab_window,
                                   GDK_OWNERSHIP_NONE,
                                   TRUE,
                                   (GdkEventMask)(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK),
                                   nullptr,
                                   GDK_CURRENT_TIME);

  if (keybGrabStatus == GDK_GRAB_SUCCESS)
    {
      GdkGrabStatus pointerGrabStatus;
      pointerGrabStatus = gdk_device_grab(pointer,
                                          grab_window,
                                          GDK_OWNERSHIP_NONE,
                                          TRUE,
                                          (GdkEventMask)(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                                                         | GDK_POINTER_MOTION_MASK),
                                          nullptr,
                                          GDK_CURRENT_TIME);

      if (pointerGrabStatus != GDK_GRAB_SUCCESS)
        {
          gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
        }
      else
        {
          ret = true;
        }
    }
#endif

  if (ret && !Platform::running_on_wayland())
    {
      xi_opcode = -1;
      GdkDisplay *display = gdk_display_get_default();
      if (GDK_IS_X11_DISPLAY(display))
        {
          Display *xdisplay = GDK_DISPLAY_XDISPLAY(display);
          int xi_event = 0;
          int xi_error = 0;
          if (XQueryExtension(xdisplay, "XInputExtension", &xi_opcode, &xi_event, &xi_error) == 0)
            {
              xi_opcode = -1;
            }
        }

      query_desktop_lock_shortcuts();
      gdk_window_add_filter(nullptr, event_filter, this);
    }

  return ret;
}

void
UnixLocker::unlock()
{
  if (!Platform::running_on_wayland())
    {
      grabbed = false;
      grab_wanted = false;
      grab_retry_connection.disconnect();

#if GTK_CHECK_VERSION(3, 24, 0)
      GdkDisplay *display = gdk_display_get_default();
      GdkSeat *seat = gdk_display_get_default_seat(display);
      gdk_seat_ungrab(seat);
      spdlog::debug("UnixLocker::unlock() ungrabbed");
#else
      if (keyboard != nullptr)
        {
          gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
          keyboard = nullptr;
        }
      if (pointer != nullptr)
        {
          gdk_device_ungrab(pointer, GDK_CURRENT_TIME);
          pointer = nullptr;
        }
#endif

      gdk_window_remove_filter(nullptr, event_filter, this);
    }
}

GdkFilterReturn
UnixLocker::event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
  auto *locker = static_cast<UnixLocker *>(data);
  auto *xev = static_cast<XEvent *>(xevent);

  if (xev->type == KeyPress)
    {
      XKeyEvent *key_event = &xev->xkey;
      spdlog::debug("Core KeyPress: keycode={} state=0x{:x} window=0x{:x}",
                    key_event->keycode,
                    key_event->state,
                    key_event->window);

      locker->handle_screen_lock_keystroke(key_event->keycode, key_event->state);
      return GDK_FILTER_CONTINUE;
    }
  if (xev->type == KeyRelease)
    {
      XKeyEvent *key_event = &xev->xkey;
      spdlog::debug("Core KeyRelease: keycode={} state=0x{:x} window=0x{:x}",
                    key_event->keycode,
                    key_event->state,
                    key_event->window);

      return GDK_FILTER_CONTINUE;
    }

  if (xev->type == GenericEvent && locker->xi_opcode != -1)
    {
      XGenericEventCookie *cookie = &xev->xcookie;
      if (cookie->extension == locker->xi_opcode && cookie->data != nullptr)
        {
          if (cookie->evtype == XI_KeyPress)
            {
              auto *xiev = static_cast<XIDeviceEvent *>(cookie->data);
              spdlog::debug("XInput2 KeyPress: detail={} mods.effective=0x{:x}", xiev->detail, xiev->mods.effective);
              locker->handle_screen_lock_keystroke(xiev->detail, xiev->mods.effective);
            }
        }
    }
  return GDK_FILTER_CONTINUE;
}

void
UnixLocker::set_default_lock_shortcuts()
{
  spdlog::info("Using default screen lock shortcuts: Super+L and Ctrl+Alt+L");
  lock_shortcuts.emplace_back(Shortcut{XK_l, Mod4Mask, true});               // Super+L
  lock_shortcuts.emplace_back(Shortcut{XK_l, ControlMask | Mod1Mask, true}); // Ctrl+Alt+L
}

void
UnixLocker::add_keybinding_shortcut(const char *binding, const char *schema_name, const char *key_name)
{
  if (binding == nullptr || binding[0] == '\0')
    {
      return;
    }

  KeySym ks = 0;
  unsigned int mods = 0;
  if (parse_keybinding(binding, &ks, &mods))
    {
      lock_shortcuts.emplace_back(Shortcut{ks, mods, true});
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

  std::vector<SchemaInfo> schemas = {{.schema_name = "org.gnome.settings-daemon.plugins.media-keys", .key_name = "screensaver"},
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

      spdlog::debug("Querying screen lock shortcuts from GSettings schema '{}'", schema_info.schema_name);

      if (!g_settings_schema_has_key(schema, schema_info.key_name))
        {
          g_object_unref(settings);
          g_settings_schema_unref(schema);
          continue;
        }

      spdlog::debug("Found '{}' key in GSettings schema '{}'", schema_info.key_name, schema_info.schema_name);

      GVariant *variant = g_settings_get_value(settings, schema_info.key_name);
      if (variant == nullptr)
        {
          g_object_unref(settings);
          g_settings_schema_unref(schema);
          continue;
        }

      spdlog::debug("GSettings key '{}' has type: {}", schema_info.key_name, g_variant_get_type_string(variant));

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

  // Convert key name to KeySym
  *keysym = XStringToKeysym(key.c_str());

  return *keysym != NoSymbol;
}

void
UnixLocker::handle_screen_lock_keystroke(unsigned int keycode, unsigned int modifier_state)
{
  GdkDisplay *display = gdk_display_get_default();
  Display *xdisplay = GDK_DISPLAY_XDISPLAY(display);
  KeySym keysym = XkbKeycodeToKeysym(xdisplay, keycode, 0, 0);

  for (const auto &shortcut: lock_shortcuts)
    {
      if (shortcut.valid && shortcut.keysym == keysym && shortcut.modifiers == modifier_state)
        {
          spdlog::info("Screen lock keystroke detected (keycode={}, keysym={}, mods=0x{:x}), locking screen",
                       keycode,
                       keysym,
                       modifier_state);
          unlock();
          System::execute(System::SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN);
          return;
        }
    }
}

bool
UnixLocker::on_lock_retry_timer()
{
  TRACE_ENTRY();
  if (grab_wanted)
    {
      lock();
    }
  return grab_wanted && !grabbed;
}
