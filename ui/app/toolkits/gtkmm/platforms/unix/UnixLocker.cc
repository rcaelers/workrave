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

#include <glibmm.h>

#include "debug.hh"
#include "utils/Platform.hh"

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
  if (!Platform::running_on_wayland())
    {
      grab_wanted = true;
      if (!grabbed)
        {
          grabbed = lock_internal();
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
  GdkGrabStatus status;

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
