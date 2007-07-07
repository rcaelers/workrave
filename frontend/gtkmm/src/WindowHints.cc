// WindowHints.cc 
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "WindowHints.hh"

#ifdef HAVE_X
#include "gnome-winhints.h"
#include "WmSpec.hh"
#endif

#include "debug.hh"

#ifdef WIN32
#include <windows.h>
#include <gdk/gdkwin32.h>
#include "harpoon.h"
#include "W32Compat.hh"
extern volatile bool HARPOON_ENABLED;
#endif

#if defined(HAVE_X)
WindowHints::hint_type WindowHints::type = WindowHints::HINTTYPE_NONE;
bool WindowHints::net_supported = false;
bool WindowHints::win_supported = false;
#endif




bool
WindowHints::init()
{
  bool rc = false;
  
#if defined(HAVE_X)
  type = HINTTYPE_NONE;

  // Check for GNOME
  gnome_win_hints_init();
  if (gnome_win_hints_wm_exists())
    {
      type = HINTTYPE_WIN;
      win_supported = true;
      rc = true;
    }

  if (WmSpec::supported())
    {
      type = HINTTYPE_NET;
      net_supported = true;
      rc = true;
    }
#endif

  return rc;
}


bool
WindowHints::set_always_on_top(GtkWidget *window, bool onTop)
{
  bool rc = false;
#ifdef HAVE_X
  if (win_supported)
    {
      gnome_win_hints_set_layer(window, onTop ? WIN_LAYER_ONTOP : WIN_LAYER_NORMAL);
      rc = true;
    }

  if (net_supported)
    {
      WmSpec::change_state(window, onTop, "_NET_WM_STATE_STAYS_ON_TOP");
      WmSpec::change_state(window, onTop, "_NET_WM_STATE_ABOVE");
      //WmSpec::change_state(window, !onTop, "_NET_WM_STATE_BELOW");
      rc = true;
    }
  
#elif defined(WIN32)
  HWND hwnd = (HWND) GDK_WINDOW_HWND(window->window);
  W32Compat::SetWindowOnTop(hwnd, onTop);
#endif
  return rc;
}


bool
WindowHints::set_skip_winlist(GtkWidget *window, bool skip)
{
  TRACE_ENTER_MSG("WindowHints:set_skip_winlist", skip);
  bool ret = false;

#ifdef HAVE_X
  switch (type)
    {
    case HINTTYPE_WIN:
      {
        unsigned int wh = gnome_win_hints_get_hints(window);
        wh &= ~(WIN_HINTS_SKIP_WINLIST | WIN_HINTS_SKIP_TASKBAR);

        if (skip)
          {
            wh |= (WIN_HINTS_SKIP_WINLIST | WIN_HINTS_SKIP_TASKBAR);
          }
            
        gnome_win_hints_set_hints(window, (GnomeWinHints) wh);
      }
      ret = true;

    case HINTTYPE_NET:
      WmSpec::change_state(window, skip, "_NET_WM_STATE_SKIP_TASKBAR");
      WmSpec::change_state(window, skip, "_NET_WM_STATE_SKIP_PAGER");
      ret = true;
      
    default:
      break;
    }
#else
  (void) window;
#endif
  TRACE_EXIT();
  return ret;
}

#ifdef WIN32
static void
win32_block_input(BOOL block, HWND *unblocked_windows)
{
  if (HARPOON_ENABLED)
    {
      if (block)
        harpoon_block_input_except_for(unblocked_windows);
      else
        harpoon_unblock_input();
    }

  UINT uPreviousState;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, block, &uPreviousState, 0);
}
#endif

//! Grabs the pointer and the keyboard.
WindowHints::Grab *
WindowHints::grab(int num_windows, GdkWindow **windows)
{
  WindowHints::Grab *handle = NULL;
#if defined(HAVE_X)
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
    }
#elif defined(WIN32)
  if (num_windows > 0)
    {
      HWND unblocked_windows[num_windows + 1];
      for (int i = 0; i < num_windows; i++)
        {
          unblocked_windows[i] = (HWND) GDK_WINDOW_HWND(windows[i]);
          SetWindowPos(unblocked_windows[i], HWND_TOPMOST,
                       0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
          BringWindowToTop(unblocked_windows[i]);
        }

      unblocked_windows[num_windows] = NULL;
      
      win32_block_input(TRUE, unblocked_windows);
      handle = (WindowHints::Grab *) 0xdeadf00d;
    }
#endif
  return handle;
}


//! Releases the pointer and keyboard grab
void
WindowHints::ungrab(WindowHints::Grab *handle)
{
  if (! handle)
    return;
  
#if defined(HAVE_X)
  gdk_keyboard_ungrab(GDK_CURRENT_TIME);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);

#elif defined(WIN32)
  win32_block_input(FALSE, NULL);
#endif
}


bool
WindowHints::set_tool_window(GtkWidget *window, bool istool)
{
  bool rc = false;
#if defined(HAVE_X)
  (void) istool;
  
  switch (type)
    {
    case HINTTYPE_NET:
      WmSpec::set_window_hint(window, "_NET_WM_WINDOW_TYPE_UTILITY");
      rc = true;
      
    default:
      break;
    }

#elif defined(WIN32)
  GdkWindow *gdkWindow = window->window;

  HWND hDrawingWind = (HWND) GDK_WINDOW_HWND(gdkWindow);
  DWORD dwExStyle = GetWindowLong (hDrawingWind, GWL_EXSTYLE);
  DWORD dwStyle = GetWindowLong (hDrawingWind, GWL_STYLE);
  if (istool)
    {
      dwExStyle |= WS_EX_TOOLWINDOW;
      dwExStyle &= ~WS_EX_APPWINDOW;
    }
  else
    {
      dwExStyle &= ~WS_EX_TOOLWINDOW;
      dwExStyle |= WS_EX_APPWINDOW;
    }
  SetWindowLong(hDrawingWind, GWL_EXSTYLE, dwExStyle);
  SetWindowLong(hDrawingWind, GWL_STYLE, dwStyle);
  rc = true;
#endif
  return rc;
}


// Rewritten by Ray Satiro
#if 0
#if defined(WIN32)
void
WindowHints::attach_thread_input(bool enabled)
{
  AttachThreadInput
    (GetWindowThreadProcessId(GetForegroundWindow(),NULL),
     GetCurrentThreadId(),enabled);
  
}
#endif
#endif
