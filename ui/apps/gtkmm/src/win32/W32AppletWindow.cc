// AppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "commonui/nls.h"
#include "debug.hh"

#include "W32AppletWindow.hh"
#include "commonui/TimerBoxControl.hh"

#if defined(interface)
#undef interface
#endif

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
  thread_abort_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
  heartbeat_data_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

  // Intentionally last line, as this one calls W32AW::set_enabled(), e.g.
  timer_box_control = new TimerBoxControl("applet", *this);

  TRACE_EXIT();
}

W32AppletWindow::~W32AppletWindow()
{
  TRACE_ENTER("W32AppletWindow::~W32AppletWindow");

  /* before this instance is destroyed we signal and wait for its worker thread to terminate. this
  isn't ideal because the gui will be blocked while we wait for termination if this destructor is
  called from the main thread. current conditions are acceptable, however. 2/12/2012
  */
  heartbeat_data.enabled = false;
  SetEvent( thread_abort_event );
  if( thread_handle )
  {
    WaitForSingleObject( thread_handle, INFINITE );
    CloseHandle( thread_handle );
  }

  if( thread_abort_event )
    CloseHandle( thread_abort_event );

  if( heartbeat_data_event )
    CloseHandle( heartbeat_data_event );

  DeleteCriticalSection(&heartbeat_data_lock);

  delete timer_box_control;

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

    update_applet_window();

    if (!menu_sent)
      {
        memcpy(&local_menu_data, &menu_data, sizeof(AppletMenuData));
        local_applet_window = applet_window;
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

void
W32AppletWindow::update_applet_window()
{
  TRACE_ENTER("W32AppletWindow::get_applet_window");
  HWND previous_applet_window = applet_window;
  if (applet_window == NULL || !IsWindow(applet_window))
    {
      HWND taskbar = FindWindow("Shell_TrayWnd",NULL);
      applet_window = RecursiveFindWindow(taskbar, APPLET_WINDOW_CLASS_NAME);
      menu_sent = false;
    }

  if (previous_applet_window == NULL && applet_window != NULL)
    {
      state_changed_signal.emit(AppletWindow::APPLET_STATE_ACTIVE);
    }
  else if (previous_applet_window != NULL && applet_window == NULL)
    {
      state_changed_signal.emit(AppletWindow::APPLET_STATE_DISABLED);
    }

  TRACE_EXIT();
}


void
W32AppletWindow::set_enabled( bool enabled )
{
  TRACE_ENTER_MSG( "W32AppletWindow::set_enabled", enabled );
  DWORD thread_exit_code = 0;

  heartbeat_data.enabled = enabled;

  if( !enabled )
    return;

  if( thread_id
    && thread_handle
    && GetExitCodeThread( thread_handle, &thread_exit_code )
    && ( thread_exit_code == STILL_ACTIVE )
  )
    return;

  if( !thread_id )
  {
    // if there is no id but a handle then this instance's worker thread has exited or is exiting.
    if( thread_handle )
      CloseHandle( thread_handle );

    thread_id = 0;
    SetLastError( 0 );
    thread_handle =
      (HANDLE)_beginthreadex( NULL, 0, run_event_pipe_static, this, 0, (unsigned int *)&thread_id );

    if( !thread_handle || !thread_id )
    {
      TRACE_MSG( "Thread could not be created. GetLastError : " << GetLastError() );
    }
  }

  TRACE_EXIT();
}


unsigned __stdcall
W32AppletWindow::run_event_pipe_static( void *param )
{
  W32AppletWindow *pThis = (W32AppletWindow *) param;
  pThis->run_event_pipe();
  // invalidate the id to signal the thread is exiting
  pThis->thread_id = 0;
  return (DWORD) 0;
}


void
W32AppletWindow::run_event_pipe()
{
  const DWORD current_thread_id = GetCurrentThreadId();

  TRACE_ENTER_MSG( "W32AppletWindow::run_event_pipe [ id: ", current_thread_id << " ]" );

  while( thread_id == current_thread_id )
  {
    /* JS: thread_abort_event must be first in the array of events.
    the index returned by WaitForMultipleObjectsEx() corresponds to the first
    signaled event in the array if more than one is signaled
    */
    HANDLE events[ 2 ] = { thread_abort_event, heartbeat_data_event };
    int const events_count = ( sizeof( events ) / sizeof( events[ 0 ] ) );

    DWORD wait_result = WaitForMultipleObjectsEx( events_count, events, FALSE, INFINITE, FALSE );

    if( ( wait_result == WAIT_FAILED ) || ( wait_result == ( WAIT_OBJECT_0 + 0 ) ) )
      break;
    
    if( heartbeat_data.enabled && ( wait_result == ( WAIT_OBJECT_0 + 1 ) ) )
    {
      EnterCriticalSection( &heartbeat_data_lock );

      update_time_bars();
      update_menu();

      LeaveCriticalSection( &heartbeat_data_lock );
    }
  }

  TRACE_EXIT();
}


void
W32AppletWindow::init_menu(HWND hwnd)
{
  menu_data.num_items = 0;
  menu_sent = false;

  /*
    As noted in ui/win32/applet/include/applet.hh:
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
  update_applet_window();
  return applet_window != NULL ? APPLET_STATE_ACTIVE : APPLET_STATE_DISABLED;
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
  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
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
