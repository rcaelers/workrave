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
#  include "config.h"
#endif

#include <stdio.h>

#include "WindowHints.hh"

#include "debug.hh"

#ifdef PLATFORM_OS_WINDOWS_NATIVE
#  undef max
#endif

#include <gtkmm/window.h>

#ifdef PLATFORM_OS_WINDOWS
#  include <windows.h>
#  include <gtk/gtk.h>
#  include <gdk/gdkwin32.h>
#  include "Harpoon.hh"
#  ifdef PLATFORM_OS_WINDOWS_NATIVE
#    undef max
#  endif
#  include <gtkmm/window.h>
#  include "harpoon.h"
#  include "W32Compat.hh"
#endif

#ifdef HAVE_GTK3
#  include "GtkUtil.hh"

#  if GTK_CHECK_VERSION(3, 24, 0)
GdkSeat *WindowHints::seat = NULL;
#  else
GdkDevice *WindowHints::keyboard = NULL;
GdkDevice *WindowHints::pointer = NULL;
#  endif
#endif

void
WindowHints::set_always_on_top(Gtk::Window *window, bool on_top)
{
#if defined(PLATFORM_OS_WINDOWS)

  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(GTK_WIDGET(window->gobj())));
  W32Compat::SetWindowOnTop(hwnd, on_top);

#else

  window->set_keep_above(on_top);

#endif
}

#ifdef PLATFORM_OS_WINDOWS
static void
win32_block_input(BOOL block)
{
  if (block)
    Harpoon::block_input();
  else
    Harpoon::unblock_input();

  UINT uPreviousState;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, block, &uPreviousState, 0);
}
#endif

//! Grabs the pointer and the keyboard.
WindowHints::Grab *
WindowHints::grab(int num_windows, GdkWindow **windows)
{
  TRACE_ENTER("WindowHints::grab");
  WindowHints::Grab *handle = NULL;

#if defined(PLATFORM_OS_WINDOWS)
  if (num_windows > 0)
    {
      HWND *unblocked_windows = new HWND[num_windows + 1];
      for (int i = 0; i < num_windows; i++)
        {
          unblocked_windows[i] = (HWND)GDK_WINDOW_HWND(windows[i]);
          SetWindowPos(unblocked_windows[i], HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
          BringWindowToTop(unblocked_windows[i]);
        }

      unblocked_windows[num_windows] = NULL;

      win32_block_input(TRUE);
      handle = (WindowHints::Grab *)0xdeadf00d;

      delete[] unblocked_windows;
    }
#elif defined(HAVE_GTK3) && GTK_CHECK_VERSION(3, 24, 0)
  // gdk_device_grab is deprecated since 3.20.
  // However, an issue that was solved in gtk 3.24 causes windows to be hidden
  // when a grab fails: https://github.com/GNOME/gtk/commit/2c8b95a518bea2192145efe11219f2e36091b37a
  if (!GtkUtil::running_on_wayland())
    {
      if (num_windows > 0)
        {
          GdkDisplay *display = gdk_window_get_display(windows[0]);
          seat = gdk_display_get_default_seat(display);

          GdkGrabStatus grabStatus = gdk_seat_grab(seat, windows[0], GDK_SEAT_CAPABILITY_ALL, TRUE, NULL, NULL, NULL, NULL);
          if (grabStatus == GDK_GRAB_SUCCESS)
            {
              // A bit of a hack, but GTK does not need any data in the handle.
              // So, let's not waste memory and simply return a bogus non-NULL ptr.
              handle = (WindowHints::Grab *)0xdeadf00d;
            }
        }
    }
#elif defined(HAVE_GTK3)
  if (!GtkUtil::running_on_wayland())
    {
      if (num_windows > 0)
        {
          GdkDevice *device = gtk_get_current_event_device();
          if (device == NULL)
            {
              GdkDisplay *display = gdk_window_get_display(windows[0]);
              GdkDeviceManager *device_manager = gdk_display_get_device_manager(display);
              device = gdk_device_manager_get_client_pointer(device_manager);
            }

          if (device != NULL)
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
                                           windows[0],
                                           GDK_OWNERSHIP_NONE,
                                           TRUE,
                                           (GdkEventMask)(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK),
                                           NULL,
                                           GDK_CURRENT_TIME);

          if (keybGrabStatus == GDK_GRAB_SUCCESS)
            {
              GdkGrabStatus pointerGrabStatus;
              pointerGrabStatus =
                gdk_device_grab(pointer,
                                windows[0],
                                GDK_OWNERSHIP_NONE,
                                TRUE,
                                (GdkEventMask)(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK),
                                NULL,
                                GDK_CURRENT_TIME);

              if (pointerGrabStatus != GDK_GRAB_SUCCESS)
                {
                  gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
                }
              else
                {
                  // A bit of a hack, but GTK does not need any data in the handle.
                  // So, let's not waste memory and simply return a bogus non-NULL ptr.
                  handle = (WindowHints::Grab *)0xdeadf00d;
                }
            }
        }
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
      pointerGrabStatus =
        gdk_pointer_grab(windows[0],
                         TRUE,
                         (GdkEventMask)(GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK),
                         NULL,
                         NULL,
                         GDK_CURRENT_TIME);

      if (pointerGrabStatus == GDK_GRAB_SUCCESS && keybGrabStatus == GDK_GRAB_SUCCESS)
        {
          // A bit of a hack, but GTK does not need any data in the handle.
          // So, let's not waste memory and simply return a bogus non-NULL ptr.
          handle = (WindowHints::Grab *)0xdeadf00d;
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
  if (!handle)
    return;

#if defined(PLATFORM_OS_WINDOWS)
  win32_block_input(FALSE);
#elif defined(HAVE_GTK3) && GTK_CHECK_VERSION(3, 24, 0)
  if (!GtkUtil::running_on_wayland())
    {
      gdk_seat_ungrab(seat);
    }
#elif defined(HAVE_GTK3)
  if (!GtkUtil::running_on_wayland())
    {
      if (keyboard != NULL)
        {
          gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
          keyboard = NULL;
        }
      if (pointer != NULL)
        {
          gdk_device_ungrab(pointer, GDK_CURRENT_TIME);
          pointer = NULL;
        }
    }
#else
  gdk_keyboard_ungrab(GDK_CURRENT_TIME);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
#endif
}
