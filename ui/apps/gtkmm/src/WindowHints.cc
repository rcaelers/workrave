// Copyright (C) 2001 - 2008, 2011, 2013 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include <stdio.h>

#include "WindowHints.hh"

#include "debug.hh"
#include "utils/Platform.hh"

#ifdef PLATFORM_OS_WIN32_NATIVE
#undef max
#endif

#include <gtkmm/window.h>

#ifdef PLATFORM_OS_WIN32
#include <windows.h>
#include <gtk/gtk.h>
#include <gdk/gdkwin32.h>
#ifdef HAVE_HARPOON
#include "harpoon.h"
#include "input-monitor/Harpoon.hh"
#endif
#ifdef PLATFORM_OS_WIN32_NATIVE
#undef max
#endif
#include <gtkmm/window.h>
#include "W32Compat.hh"
#endif

GdkDevice *WindowHints::keyboard = nullptr;
GdkDevice *WindowHints::pointer = nullptr;

void
WindowHints::set_always_on_top(Gtk::Window *window, bool on_top)
{
#if defined(PLATFORM_OS_WIN32)

  HWND hwnd = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(GTK_WIDGET(window->gobj())));
  W32Compat::SetWindowOnTop(hwnd, on_top);

#else

  window->set_keep_above(on_top);

#endif
}


bool
WindowHints::can_grab()
{
#ifdef PLATFORM_OS_WIN32
#ifdef HAVE_HARPOON
  return true;
#else
  return false;
#endif
#elif PLATFORM_OS_UNIX
  return !Platform::running_on_wayland();
#else
  return false;
#endif
}


#ifdef PLATFORM_OS_WIN32
static void
win32_block_input(BOOL block)
{
#ifdef HAVE_HARPOON
  if (block)
    Harpoon::block_input();
  else
    Harpoon::unblock_input();
#endif

  UINT uPreviousState;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, block, &uPreviousState, 0);
}
#endif


//! Grabs the pointer and the keyboard.
WindowHints::Grab *
WindowHints::grab(int num_windows, GdkWindow **windows)
{
  TRACE_ENTER("WindowHints::grab");
  WindowHints::Grab *handle = nullptr;

#if defined(PLATFORM_OS_WIN32)
  if (num_windows > 0)
    {
      HWND *unblocked_windows = new HWND[num_windows + 1];
      for (int i = 0; i < num_windows; i++)
        {
          unblocked_windows[i] = (HWND) GDK_WINDOW_HWND(windows[i]);
          SetWindowPos(unblocked_windows[i], HWND_TOPMOST,
                       0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
          BringWindowToTop(unblocked_windows[i]);
        }

      unblocked_windows[num_windows] = NULL;

      win32_block_input(TRUE);
      handle = (WindowHints::Grab *) 0xdeadf00d;

      delete [] unblocked_windows;
    }
#elif defined(HAVE_GTK)
  if (num_windows > 0)
    {
#if GTK_CHECK_VERSION(3, 20, 0)
      GdkGrabStatus status;

      GdkDisplay *display = gdk_display_get_default();
      GdkSeat *seat = gdk_display_get_default_seat(display);
      status = gdk_seat_grab(seat, windows[0], GDK_SEAT_CAPABILITY_ALL, TRUE, NULL, NULL, NULL, NULL);

      if (status == GDK_GRAB_SUCCESS)
        {
          // A bit of a hack, but GTK does not need any data in the handle.
          // So, let's not waste memory and simply return a bogus non-NULL ptr.
          handle = (WindowHints::Grab *) 0xdeadf00d;
        }
#else
      GdkDevice *device = gtk_get_current_event_device();
      if (device == nullptr)
        {
          GdkDisplay *display = gdk_window_get_display(windows[0]);
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
      keybGrabStatus = gdk_device_grab(keyboard, windows[0],
                                       GDK_OWNERSHIP_NONE, TRUE,
                                       (GdkEventMask) (GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK),
                                       nullptr, GDK_CURRENT_TIME);

      if (keybGrabStatus == GDK_GRAB_SUCCESS)
        {
          GdkGrabStatus pointerGrabStatus;
          pointerGrabStatus = gdk_device_grab(pointer, windows[0],
                                              GDK_OWNERSHIP_NONE, TRUE,
                                              (GdkEventMask) (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                                              nullptr, GDK_CURRENT_TIME);

          if (pointerGrabStatus != GDK_GRAB_SUCCESS)
            {
              gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
            }
          else
            {
              // A bit of a hack, but GTK does not need any data in the handle.
              // So, let's not waste memory and simply return a bogus non-NULL ptr.
              handle = (WindowHints::Grab *) 0xdeadf00d;
            }
        }
#endif
    }
#else
  if (num_windows > 0)
    {
      // Only grab first window.

      // Grab keyboard.
      GdkGrabStatus keybGrabStatus;
      keybGrabStatus = gdk_keyboard_grab(windows[0], TRUE, GDK_CURRENT_TIME);

      // Grab pointer
      GdkGrabStatus pointerGrabStatus;
      pointerGrabStatus = gdk_pointer_grab(windows[0],
                                           TRUE,
                                           (GdkEventMask) (GDK_BUTTON_RELEASE_MASK |
                                                           GDK_BUTTON_PRESS_MASK |
                                                           GDK_POINTER_MOTION_MASK),
                                           NULL, NULL, GDK_CURRENT_TIME);

      if (pointerGrabStatus == GDK_GRAB_SUCCESS
          && keybGrabStatus == GDK_GRAB_SUCCESS)
        {
          // A bit of a hack, but GTK does not need any data in the handle.
          // So, let's not waste memory and simply return a bogus non-NULL ptr.
          handle = (WindowHints::Grab *) 0xdeadf00d;
        }
      else
        {
          // Ungrab both
          gdk_keyboard_ungrab(GDK_CURRENT_TIME);
          gdk_pointer_ungrab(GDK_CURRENT_TIME);
        }

      TRACE_MSG(keybGrabStatus << " " << pointerGrabStatus);
    }

#endif
  TRACE_EXIT();
  return handle;
}


//! Releases the pointer and keyboard grab
void
WindowHints::ungrab(WindowHints::Grab *handle)
{
  if (! handle)
    return;

#if defined(PLATFORM_OS_WIN32)
  win32_block_input(FALSE);
#elif defined(HAVE_GTK)
#  if GTK_CHECK_VERSION(3, 20, 0)
  GdkDisplay *display = gdk_display_get_default();
  GdkSeat *seat = gdk_display_get_default_seat(display);
  gdk_seat_ungrab(seat);
#  else
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
#  endif
#else
  gdk_keyboard_ungrab(GDK_CURRENT_TIME);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
#endif
}
