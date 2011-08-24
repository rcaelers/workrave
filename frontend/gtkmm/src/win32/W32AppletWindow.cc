// AppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "W32AppletWindow.hh"
#include "TimerBoxControl.hh"
#include "Applet.hh"
#include "GUI.hh"
#include "Menus.hh"

W32AppletWindow::W32AppletWindow()
{
  TRACE_ENTER("W32AppletWindow::W32AppletWindow");

  memset(&local_heartbeat_data, 0, sizeof(AppletHeartbeatData));
  memset(&local_menu_data, 0, sizeof(AppletMenuData));
  memset(&heartbeat_data, 0, sizeof(AppletHeartbeatData));
  memset(&menu_data, 0, sizeof(AppletMenuData));

  thread_id = 0;
  thread_handle = NULL;
  timer_box_view = this;
  applet_window = NULL;
  heartbeat_data.enabled = false;
  local_applet_window = NULL;
  init_menu(NULL);

  ::InitializeCriticalSection(&heartbeat_data_lock);
  heartbeat_data_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

  // Intentionally last line, as this one calls W32AW::set_enabled(), e.g.
  timer_box_control = new TimerBoxControl("applet", *this);

  TRACE_EXIT();
}

W32AppletWindow::~W32AppletWindow()
{
  TRACE_ENTER("W32AppletWindow::~W32AppletWindow");
  delete timer_box_control;

  ::DeleteCriticalSection(&heartbeat_data_lock);
  CloseHandle(heartbeat_data_event);

  TRACE_EXIT();
}


static HWND
RecursiveFindWindow(HWND hwnd, LPCTSTR lpClassName)
{
  static char buf[80];
  int num = GetClassName(hwnd, buf, sizeof(buf)-1);
  buf[num] = 0;
  HWND ret = NULL;

  if (! stricmp(lpClassName, buf))
    {
      ret =  hwnd;
    }
  else
    {
      HWND child = FindWindowEx(hwnd, 0, NULL, NULL);
      while (child != NULL)
        {
          ret = RecursiveFindWindow(child, lpClassName);
          if (ret)
            {
              break;
            }
          child = FindWindowEx(hwnd, child, NULL, NULL);
        }
    }
  return ret;
}



void
W32AppletWindow::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_slot", int(id) << ", " << slot);
  heartbeat_data.slots[slot] = (short) id;
  TRACE_EXIT();
}

void
W32AppletWindow::set_time_bar(BreakId id,
                              std::string text,
                              ITimeBar::ColorId primary_color,
                              int primary_val, int primary_max,
                              ITimeBar::ColorId secondary_color,
                              int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_time_bar", int(id) << "=" << text);
  strncpy(heartbeat_data.bar_text[id], text.c_str(), APPLET_BAR_TEXT_MAX_LENGTH-1);
  heartbeat_data.bar_text[id][APPLET_BAR_TEXT_MAX_LENGTH-1] = '\0';
  heartbeat_data.bar_primary_color[id] = primary_color;
  heartbeat_data.bar_primary_val[id] = primary_val;
  heartbeat_data.bar_primary_max[id] = primary_max;
  heartbeat_data.bar_secondary_color[id] = secondary_color;
  heartbeat_data.bar_secondary_val[id] = secondary_val;
  heartbeat_data.bar_secondary_max[id] = secondary_max;
  TRACE_EXIT();
}

void
W32AppletWindow::update_view()
{
  TRACE_ENTER("W32AppletWindow::update_view");

  BOOL entered = ::TryEnterCriticalSection(&heartbeat_data_lock);
  if (entered)
  {
    memcpy(&local_heartbeat_data, &heartbeat_data, sizeof(AppletHeartbeatData));

    HWND window = get_applet_window();

    if (!menu_sent)
      {
        memcpy(&local_menu_data, &menu_data, sizeof(AppletMenuData));
        local_applet_window = window;
        menu_sent = true;
      }

    SetEvent(heartbeat_data_event);
    ::LeaveCriticalSection(&heartbeat_data_lock);
  }

  TRACE_EXIT();
}

void
W32AppletWindow::update_menu()
{
  TRACE_ENTER("W32AppletWindow::update_menu");
  if (local_applet_window != NULL)
    {
      TRACE_MSG("sending");

      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_MENU;
      msg.cbData = sizeof(AppletMenuData);
      msg.lpData = &local_menu_data;
      SendMessage(local_applet_window, WM_COPYDATA, 0, (LPARAM) &msg);
    }
  TRACE_EXIT();
}

void
W32AppletWindow::update_time_bars()
{
  TRACE_ENTER("W32AppletWindow::update_time_bars");
  if (local_applet_window != NULL)
    {
      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_HEARTBEAT;
      msg.cbData = sizeof(AppletHeartbeatData);
      msg.lpData = &local_heartbeat_data;
      TRACE_MSG("sending: enabled=" << local_heartbeat_data.enabled);
      for (size_t i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          TRACE_MSG("sending: slots[]=" << local_heartbeat_data.slots[i]);
        }
      SendMessage(local_applet_window, WM_COPYDATA, 0, (LPARAM) &msg);
    }
  TRACE_EXIT();
}

HWND
W32AppletWindow::get_applet_window()
{
  TRACE_ENTER("W32AppletWindow::get_applet_window");
  if (applet_window == NULL || !IsWindow(applet_window))
    {
      HWND taskbar = FindWindow("Shell_TrayWnd",NULL);
      applet_window = RecursiveFindWindow(taskbar, APPLET_WINDOW_CLASS_NAME);
      menu_sent = false;
    }
  TRACE_RETURN((applet_window ? "Applet found" : "Applet not found"));
  return applet_window;
}


void
W32AppletWindow::set_enabled(bool enabled)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_enabled", enabled);
  heartbeat_data.enabled = enabled;

  if (enabled)
    {
      thread_id = 0;
      thread_handle = CreateThread(NULL, 0, run_event_pipe_static, this, 0, (DWORD *)&thread_id);

      if (thread_handle == NULL || thread_id == 0)
        {
          TRACE_MSG( "Thread could not be created. GetLastError : " << GetLastError());
          TRACE_EXIT();
        }

      CloseHandle(thread_handle);
      thread_handle = NULL;
    }

  TRACE_EXIT();
}


DWORD WINAPI
W32AppletWindow::run_event_pipe_static(LPVOID lpParam)
{
  W32AppletWindow *pThis = (W32AppletWindow *) lpParam;
  pThis->run_event_pipe();
  return (DWORD) 0;
}


void
W32AppletWindow::run_event_pipe()
{
  while (heartbeat_data.enabled)
    {
      DWORD wait_result = ::WaitForSingleObjectEx(heartbeat_data_event,
                                                  INFINITE,
                                                  FALSE);
      if (wait_result != WAIT_FAILED)
        {
          ::EnterCriticalSection(&heartbeat_data_lock);

          update_time_bars();
          update_menu();

          ::LeaveCriticalSection(&heartbeat_data_lock);
        }
    }
}


void
W32AppletWindow::init_menu(HWND hwnd)
{
  menu_data.num_items = 0;
  menu_sent = false;

  /*
    As noted in frontend/win32/applet/include/applet.hh:
    We pass the command_window HWND as a LONG for compatibility.
  */
  menu_data.command_window = HandleToLong( hwnd );
}

void
W32AppletWindow::add_menu(const char *text, short cmd, int flags)
{
  AppletMenuItemData *d = &menu_data.items[menu_data.num_items++];
  d->command = cmd;
  strcpy(d->text, text);
  d->flags = flags;
}

AppletWindow::AppletState
W32AppletWindow::activate_applet()
{
  return APPLET_STATE_VISIBLE;
}


void
W32AppletWindow::deactivate_applet()
{
}


void
W32AppletWindow::set_geometry(Orientation orientation, int size)
{
  (void) orientation;
  (void) size;
}


bool
W32AppletWindow::on_applet_command(int command)
{
  TRACE_ENTER_MSG("W32AppletWindow::on_applet_command", command);
  GUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();;
  menus->applet_command(command);
  TRACE_EXIT();
  return false;
}


GdkFilterReturn
W32AppletWindow::win32_filter_func (void     *xevent,
                                    GdkEvent *event)
{
  (void) event;
  MSG *msg = (MSG *) xevent;
  GdkFilterReturn ret = GDK_FILTER_CONTINUE;

  switch (msg->message)
    {
    case WM_USER:
      {
        sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this, &W32AppletWindow::on_applet_command),
                                              (int) msg->wParam);
        Glib::signal_idle().connect(my_slot);

        ret = GDK_FILTER_REMOVE;
      }
      break;

    case WM_USER + 1:
      {
        timer_box_control->force_cycle();
        ret = GDK_FILTER_REMOVE;
      }
      break;
    }
  return ret;
}
