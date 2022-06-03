/*
 * Copyright (C) 2002-2008 Raymond Penners <raymond@dotsphinx.com>
 * Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
 * Copyright (C) 2009-2010 Rob Caelers <robc@krandor.nl>
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>

#include "harpoon.h"

#if !defined(WH_KEYBOARD_LL)
#  error WH_KEYBOARD_LL not defined.
#endif

#if defined(__GNUC__)
#  define DLLSHARE(v) v __attribute__((section(".shared"), shared))
#  if !defined(INLINE)
#    define INLINE inline
#  endif
#else
#  if !defined(INLINE)
#    define INLINE
#  endif
#  define snprintf _snprintf
#  define DLLSHARE(v) v
#  pragma comment(linker, "/SECTION:.shared,RWS")
#  pragma data_seg(".shared")
#endif
HWND DLLSHARE(notification_window) = NULL;
HHOOK DLLSHARE(mouse_hook) = NULL;
HHOOK DLLSHARE(mouse_ll_hook) = NULL;
HHOOK DLLSHARE(keyboard_hook) = NULL;
HHOOK DLLSHARE(keyboard_ll_hook) = NULL;
HHOOK DLLSHARE(msg_hook) = NULL;
BOOL DLLSHARE(block_input) = FALSE;
BOOL DLLSHARE(initialized) = FALSE;
char DLLSHARE(critical_file_list[HARPOON_MAX_UNBLOCKED_APPS][511]) = {
  0,
};
HWND DLLSHARE(debug_hwnd) = NULL;
int DLLSHARE(debug) = FALSE;

#if !defined(__GNUC__)
#  pragma data_seg()
#endif

#define IDM_MENU_SAVE 0
#define IDM_MENU_MONITOR 1
#define IDM_MENU_CLEAR 2
#define IDM_MENU_UNHOOK 3

// Each instance of harpoon should have the
// filename of the process it is attached to.
static unsigned char exec_filename[511];
static int exec_filename_critical = FALSE;
static int exec_filename_workrave = FALSE;
static void _get_exec_filename(void);
static DWORD exec_process_id = 0;

static void debug_send_message(const char *);
static INLINE void if_debug_send_message(const char *);
static void debug_process_menu_selection(WORD);
static void debug_save_data();
static HMENU menu = NULL;

static HANDLE dll_handle = NULL;
static volatile HarpoonHookFunc user_callback = NULL;
static ATOM notification_class = 0;

#if (_WIN32_WINNT < 0x0500)
typedef struct
{
  MOUSEHOOKSTRUCT MOUSEHOOKSTRUCT;
  DWORD mouseData;
} MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;
#endif

static void harpoon_hook_block_only(void);

/**********************************************************************
 * Misc
 **********************************************************************/

#ifdef GRAVEYARD

static DWORD
harpoon_get_service_pack()
{
  HKEY hKey;
  DWORD dwCSDVersion;
  DWORD dwSize;
  DWORD ret = 0;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\Windows", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
      dwSize = sizeof(dwCSDVersion);
      if (RegQueryValueEx(hKey, "CSDVersion", NULL, NULL, (unsigned char *)&dwCSDVersion, &dwSize) == ERROR_SUCCESS)
        {
          ret = (LOWORD(dwCSDVersion));
        }
      RegCloseKey(hKey);
    }
  return ret;
}

#endif

/**********************************************************************
 * Blocking
 **********************************************************************/

HARPOON_API void
harpoon_unblock_input(void)
{
  block_input = FALSE;
  if_debug_send_message("block_input = FALSE");

  if (user_callback == NULL && initialized)
    {
      harpoon_unhook();
    }
}

HARPOON_API void
harpoon_block_input(void)
{
  block_input = TRUE;
  if_debug_send_message("block_input = TRUE");

  if (user_callback == NULL && initialized)
    {
      harpoon_hook_block_only();
    }
}

static int
is_app_blocked()
// Ensures we don't block any critical applications
{
  int i;
  static int runonce = TRUE;

  if (runonce)
    {
      runonce = FALSE;
      /*
      We only need to get the filename once per process.
      Workrave already has exec_filename_workrave set TRUE.
      See harpoon hook/init functions.
      */
      if (exec_filename_workrave)
        exec_filename_critical = TRUE;

      _get_exec_filename();
      // exec_filename[]should now contain the process filename

      for (i = 0; i < HARPOON_MAX_UNBLOCKED_APPS; ++i)
        if (strncmp((char *)exec_filename, critical_file_list[i], 510) == 0)
          exec_filename_critical = TRUE;

      if (exec_filename_critical)
        if_debug_send_message("-->Harpoon hooked (exec_filename_critical == TRUE)");
      else
        if_debug_send_message("-->Harpoon hooked");
    }

  if (exec_filename_critical)
    // don't block input to critical app
    return FALSE;
  else
    return TRUE;
}

static INLINE LRESULT
harpoon_generic_hook_return(int code, WPARAM wpar, LPARAM lpar, HHOOK hook, BOOL forcecallnext)
{
  // note: make sure is_app_blocked() is evaluated second.
  // this way we can get the instance's process name asap.
  if (is_app_blocked() && !forcecallnext && block_input && code == HC_ACTION)
    // block mouse input:
    return (LRESULT)-1;
  else
    return CallNextHookEx(hook, code, wpar, lpar);
}

/**********************************************************************
 * Messaging
 **********************************************************************/

static INLINE void
harpoon_post_message(HarpoonEventType evt, int par1, int par2)
{
  PostMessage(notification_window, WM_USER + evt, (WPARAM)par1, (LPARAM)par2);
}

static LRESULT CALLBACK
harpoon_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

  if (debug_hwnd)
    {
      if (HIWORD(wParam) == 0 && lParam == 0 && uMsg == WM_COMMAND)
        // Menu item selected
        debug_process_menu_selection(LOWORD(wParam));

      else if (uMsg == WM_SIZE && wParam != SIZE_MINIMIZED)
        // If the notification window is resized, resize debug_hwnd.
        {
          MoveWindow(debug_hwnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
          if_debug_send_message("WM_SIZE: notification_window resized.");
        }
      else if (uMsg == WM_SYSCOMMAND && wParam == SC_CLOSE)
        // Prevent the user from closing the notification window.
        {
          if_debug_send_message("The notification window cannot be closed.");
          if_debug_send_message("Preferences > Advanced > harpoon > Disable Debug");
          return (LRESULT)-1;
        }
    }

  if (user_callback)
    {
      HarpoonEvent evt;
      int evt_type;
      evt.type = HARPOON_NOTHING;
      evt_type = uMsg - WM_USER;
      if (evt_type >= 0 && evt_type < HARPOON_EVENT__SIZEOF)
        {
          evt.type = (HarpoonEventType)evt_type;
          switch (evt.type)
            {
            case HARPOON_KEY_PRESS:
            case HARPOON_KEY_RELEASE:
              evt.keyboard.flags = (int)lParam;
              break;

            case HARPOON_BUTTON_PRESS:
            case HARPOON_BUTTON_RELEASE:
            case HARPOON_2BUTTON_PRESS:
            case HARPOON_MOUSE_WHEEL:
            case HARPOON_MOUSE_MOVE:
              /*
              The x and y mouse coordinates are packed into lParam.
              Here we separate x and y. It's important to cast as
              signed, because the coordinate(s) could be negative.
              Casting to the signed type allows the compiler to
              properly promote the signed short to a signed int.
              */
              evt.mouse.x = (short)LOWORD(lParam);
              evt.mouse.y = (short)HIWORD(lParam);
              if (evt.type == HARPOON_MOUSE_WHEEL)
                {
                  evt.mouse.button = -1;
                  evt.mouse.wheel = (int)wParam;
                }
              else
                {
                  evt.mouse.button = (int)wParam;
                  evt.mouse.wheel = 0;
                }
              break;

            default:
              evt.type = HARPOON_NOTHING;
            }

          if (evt.type != HARPOON_NOTHING)
            {
              (*user_callback)(&evt);
            }
        }
    }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**********************************************************************
 * Mouse hook
 **********************************************************************/

static LRESULT CALLBACK
harpoon_mouse_hook(int code, WPARAM wpar, LPARAM lpar)
{
  BOOL forcecallnext = FALSE;

  if (code == HC_ACTION)
    {
      PMOUSEHOOKSTRUCT pmhs = (PMOUSEHOOKSTRUCT)lpar;
      HarpoonEventType evt = HARPOON_NOTHING;
      int button = -1;
      int x = pmhs->pt.x;
      int y = pmhs->pt.y;

      // If WH_MOUSE_LL is hooked,
      // WH_MOUSE messages are not appended to the debug window.
      // This is mainly to avoid overflow.
      switch (wpar)
        {
        case WM_NCLBUTTONDOWN:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCLBUTTONDOWN");
        case WM_LBUTTONDOWN:
          button = 0;
          evt = HARPOON_BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_LBUTTONDOWN)
            debug_send_message("WH_MOUSE: WM_LBUTTONDOWN");
          break;
        case WM_NCMBUTTONDOWN:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCMBUTTONDOWN");
        case WM_MBUTTONDOWN:
          button = 1;
          evt = HARPOON_BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_MBUTTONDOWN)
            debug_send_message("WH_MOUSE: WM_MBUTTONDOWN");
          break;
        case WM_NCRBUTTONDOWN:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCRBUTTONDOWN");
        case WM_RBUTTONDOWN:
          button = 2;
          evt = HARPOON_BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_RBUTTONDOWN)
            debug_send_message("WH_MOUSE: WM_RBUTTONDOWN");
          break;
        case WM_NCXBUTTONDOWN:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCXBUTTONDOWN");
        case WM_XBUTTONDOWN:
          button = (HIWORD(wpar) == XBUTTON1) ? 3 : 4;
          evt = HARPOON_BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_XBUTTONDOWN)
            debug_send_message("WH_MOUSE: WM_XBUTTONDOWN");
          break;

        case WM_NCLBUTTONUP:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCLBUTTONUP");
        case WM_LBUTTONUP:
          button = 0;
          evt = HARPOON_BUTTON_RELEASE;
          if (debug && !mouse_ll_hook && wpar == WM_LBUTTONUP)
            debug_send_message("WH_MOUSE: WM_LBUTTONUP");
          break;
        case WM_NCMBUTTONUP:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCMBUTTONUP");
        case WM_MBUTTONUP:
          button = 1;
          evt = HARPOON_BUTTON_RELEASE;
          if (debug && !mouse_ll_hook && wpar == WM_MBUTTONUP)
            debug_send_message("WH_MOUSE: WM_MBUTTONUP");
          break;
        case WM_NCRBUTTONUP:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCRBUTTONUP");
        case WM_RBUTTONUP:
          button = 2;
          evt = HARPOON_BUTTON_RELEASE;
          if (debug && !mouse_ll_hook && wpar == WM_RBUTTONUP)
            debug_send_message("WH_MOUSE: WM_RBUTTONUP");
          break;
        case WM_NCXBUTTONUP:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCXBUTTONUP");
        case WM_XBUTTONUP:
          button = (HIWORD(wpar) == XBUTTON1) ? 3 : 4;
          evt = HARPOON_BUTTON_RELEASE;
          if (debug && !mouse_ll_hook && wpar == WM_XBUTTONUP)
            debug_send_message("WH_MOUSE: WM_XBUTTONUP");
          break;

        case WM_NCLBUTTONDBLCLK:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCLBUTTONDBLCLK");
        case WM_LBUTTONDBLCLK:
          button = 0;
          evt = HARPOON_2BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_LBUTTONDBLCLK)
            debug_send_message("WH_MOUSE: WM_LBUTTONDBLCLK");
          break;
        case WM_NCMBUTTONDBLCLK:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCMBUTTONDBLCLK");
        case WM_MBUTTONDBLCLK:
          button = 1;
          evt = HARPOON_2BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_MBUTTONDBLCLK)
            debug_send_message("WH_MOUSE: WM_MBUTTONDBLCLK");
          break;
        case WM_NCRBUTTONDBLCLK:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCRBUTTONDBLCLK");
        case WM_RBUTTONDBLCLK:
          button = 2;
          evt = HARPOON_2BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_RBUTTONDBLCLK)
            debug_send_message("WH_MOUSE: WM_RBUTTONDBLCLK");
          break;
        case WM_NCXBUTTONDBLCLK:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCXBUTTONDBLCLK");
        case WM_XBUTTONDBLCLK:
          button = (HIWORD(wpar) == XBUTTON1) ? 3 : 4;
          evt = HARPOON_2BUTTON_PRESS;
          if (debug && !mouse_ll_hook && wpar == WM_XBUTTONDBLCLK)
            debug_send_message("WH_MOUSE: WM_XBUTTONDBLCLK");
          break;

        case WM_MOUSEWHEEL:
          evt = HARPOON_MOUSE_WHEEL;
          button = 1;
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_MOUSEWHEEL");
          break;
#ifdef WM_MOUSEHWHEEL
        case WM_MOUSEHWHEEL:
          evt = HARPOON_MOUSE_WHEEL;
          button = 2;
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_MOUSEHWHEEL");
          break;
#endif
        case WM_NCMOUSEMOVE:
          if (debug && !mouse_ll_hook)
            debug_send_message("WH_MOUSE: WM_NCMOUSEMOVE");
        case WM_MOUSEMOVE:
          evt = HARPOON_MOUSE_MOVE;
          if (debug && !mouse_ll_hook && wpar == WM_MOUSEMOVE)
            debug_send_message("WH_MOUSE: WM_MOUSEMOVE");
        }

      /*
   The low-level mouse hook is always preferred over the
   regular mouse hook. The low-level hook posts its own
   message to the notification window. Here, we check to
   see if there is a low-level hook. If not, we post our
   own message.
   */
      if (!mouse_ll_hook && evt != HARPOON_NOTHING)
        harpoon_post_message(evt, button, MAKELONG(x, y));

      if (evt == HARPOON_BUTTON_RELEASE)
        {
          forcecallnext = TRUE;
        }
    }
  return harpoon_generic_hook_return(code, wpar, lpar, mouse_hook, forcecallnext);
}

static LRESULT CALLBACK
harpoon_mouse_ll_hook(int code, WPARAM wpar, LPARAM lpar)
{
  if (code == HC_ACTION)
    {
      PMSLLHOOKSTRUCT pmhs = (PMSLLHOOKSTRUCT)lpar;
      HarpoonEventType evt = HARPOON_NOTHING;
      int button = -1;
      int x = pmhs->pt.x;
      int y = pmhs->pt.y;

      switch (wpar)
        {
        case WM_LBUTTONDOWN:
          button = 0;
          evt = HARPOON_BUTTON_PRESS;
          if_debug_send_message("WH_MOUSE_LL: WM_LBUTTONDOWN");
          break;
        case WM_MBUTTONDOWN:
          button = 1;
          evt = HARPOON_BUTTON_PRESS;
          if_debug_send_message("WH_MOUSE_LL: WM_MBUTTONDOWN");
          break;
        case WM_RBUTTONDOWN:
          button = 2;
          evt = HARPOON_BUTTON_PRESS;
          if_debug_send_message("WH_MOUSE_LL: WM_RBUTTONDOWN");
          break;
        case WM_XBUTTONDOWN:
          button = (HIWORD(wpar) == XBUTTON1) ? 3 : 4;
          evt = HARPOON_BUTTON_PRESS;
          if_debug_send_message("WH_MOUSE_LL: WM_XBUTTONDOWN");
          break;

        case WM_LBUTTONUP:
          button = 0;
          evt = HARPOON_BUTTON_RELEASE;
          if_debug_send_message("WH_MOUSE_LL: WM_LBUTTONUP");
          break;
        case WM_MBUTTONUP:
          button = 1;
          evt = HARPOON_BUTTON_RELEASE;
          if_debug_send_message("WH_MOUSE_LL: WM_MBUTTONUP");
          break;
        case WM_RBUTTONUP:
          button = 2;
          evt = HARPOON_BUTTON_RELEASE;
          if_debug_send_message("WH_MOUSE_LL: WM_RBUTTONUP");
          break;
        case WM_XBUTTONUP:
          button = (HIWORD(wpar) == XBUTTON1) ? 3 : 4;
          evt = HARPOON_BUTTON_RELEASE;
          if_debug_send_message("WH_MOUSE_LL: WM_XBUTTONUP");
          break;

        case WM_MOUSEWHEEL:
          evt = HARPOON_MOUSE_WHEEL;
          button = 1;
          if_debug_send_message("WH_MOUSE_LL: WM_MOUSEWHEEL");
          break;
#ifdef WM_MOUSEHWHEEL
        case WM_MOUSEHWHEEL:
          evt = HARPOON_MOUSE_WHEEL;
          button = 2;
          if_debug_send_message("WH_MOUSE_LL: WM_MOUSEHWHEEL");
          break;
#endif
        case WM_MOUSEMOVE:
          evt = HARPOON_MOUSE_MOVE;
          if_debug_send_message("WH_MOUSE_LL: WM_MOUSEMOVE");
        }

      if (evt != HARPOON_NOTHING)
        harpoon_post_message(evt, button, MAKELONG(x, y));
    }

  return harpoon_generic_hook_return(code, wpar, lpar, mouse_ll_hook, TRUE);
}

#if !defined(_WIN64)
static LRESULT CALLBACK
harpoon_mouse_block_hook(int code, WPARAM wpar, LPARAM lpar)
{
  BOOL forcecallnext = FALSE;

  if (code == HC_ACTION)
    {
      if_debug_send_message("WH_MOUSE");

      switch (wpar)
        {
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_XBUTTONUP:
          forcecallnext = TRUE;
        }
    }

  return harpoon_generic_hook_return(code, wpar, lpar, mouse_hook, forcecallnext);
}
#endif

/**********************************************************************
 * Keyboard hook
 **********************************************************************/

#ifdef GRAVEYARD
static BOOL
harpoon_supports_keyboard_ll(void)
{
  OSVERSIONINFO info;
  BOOL ret = FALSE;
  if (GetVersionEx(&info))
    {
      if (info.dwPlatformId > VER_PLATFORM_WIN32_NT)
        {
          ret = TRUE;
        }
      else if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
          /* Check for min. SP3. */
          DWORD sp = harpoon_get_service_pack();
          ret = (sp >= 0x300);
        }
    }
  return ret;
}
#endif

static LRESULT CALLBACK
harpoon_keyboard_hook(int code, WPARAM wpar, LPARAM lpar)
{
  BOOL forcecallnext = FALSE;
  if (code == HC_ACTION)
    {
      BOOL pressed = (lpar & (1 << 31)) == 0;
      BOOL prevpressed = (lpar & (1 << 30)) != 0;
      HarpoonEventType evt;
      int flags = 0;
      if (pressed && prevpressed)
        {
          flags |= HARPOON_KEY_REPEAT_FLAG;
        }

      evt = pressed ? HARPOON_KEY_PRESS : HARPOON_KEY_RELEASE;
      forcecallnext = !pressed;

      /*
        The low-level keyboard hook is always preferred over the
        regular keyboard hook. The low-level hook posts its own
        message to the notification window. Here, we check to
        see if there is a low-level hook. If not, we post our
        own message.
      */
      if (!keyboard_ll_hook)
        {
          // The low level hook also intercepts keys injected using keybd_event.
          // Some application use this function to toggle the keyboard lights...
          if (wpar != VK_NUMLOCK && wpar != VK_CAPITAL && wpar != VK_SCROLL)
            {
              harpoon_post_message(evt, 0, flags);
            }
        }
      if_debug_send_message("WH_KEYBOARD");
    }
  return harpoon_generic_hook_return(code, wpar, lpar, keyboard_hook, forcecallnext);
}

static LRESULT CALLBACK
harpoon_keyboard_ll_hook(int code, WPARAM wpar, LPARAM lpar)
{
  BOOL forcecallnext = FALSE;
  if (code == HC_ACTION)
    {
      KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *)lpar;
      BOOL pressed = !(kb->flags & (1 << 7));
      forcecallnext = !pressed;

      // The low level hook also intercepts keys injected using keybd_event.
      // Some application use this function to toggle the keyboard lights...
      if (kb->vkCode != VK_NUMLOCK && kb->vkCode != VK_CAPITAL && kb->vkCode != VK_SCROLL)
        {
          HarpoonEventType evt = pressed ? HARPOON_KEY_PRESS : HARPOON_KEY_RELEASE;
          harpoon_post_message(evt, 0, 0);
          if_debug_send_message("WH_KEYBOARD_LL");
        }
    }
  return harpoon_generic_hook_return(code, wpar, lpar, keyboard_ll_hook, forcecallnext);
}

#if !defined(_WIN64)
static LRESULT CALLBACK
harpoon_keyboard_block_hook(int code, WPARAM wpar, LPARAM lpar)
{
  BOOL forcecallnext = FALSE;
  if (code == HC_ACTION)
    {
      BOOL pressed = (lpar & (1 << 31)) == 0;
      forcecallnext = !pressed;
      if_debug_send_message("WH_KEYBOARD");
    }
  return harpoon_generic_hook_return(code, wpar, lpar, keyboard_hook, forcecallnext);
}
#endif

#if defined(_WIN64)
static LRESULT CALLBACK
harpoon_msg_block_hook(int code, WPARAM wpar, LPARAM lpar)
{
  if (code >= 0)
    {
      // if (is_app_blocked() && block_input)
      //{
      //   ((MSG*)lpar)->message = WM_NULL;
      //}

      if (((MSG *)lpar)->message == WM_KEYDOWN)
        {
          if (is_app_blocked() && block_input)
            {
              ((MSG *)lpar)->message = WM_NULL;
            }
        }

      if (((MSG *)lpar)->message >= WM_MOUSEFIRST && ((MSG *)lpar)->message <= WM_MOUSELAST)
        {
          if (is_app_blocked() && block_input)
            {
              ((MSG *)lpar)->message = WM_NULL;
            }
        }

      if (((MSG *)lpar)->message >= WM_NCMOUSEMOVE && ((MSG *)lpar)->message <= WM_NCMOUSEMOVE + 0x10)
        {
          if (is_app_blocked() && block_input)
            {
              ((MSG *)lpar)->message = WM_NULL;
            }
        }
    }

  return CallNextHookEx(msg_hook, code, wpar, lpar);
}
#endif

/**********************************************************************
 * Initialisation
 **********************************************************************/

HARPOON_API BOOL
harpoon_init(char imported_critical_file_list[][511], BOOL debug_harpoon)
{
  int i;
  RECT rect;
  HMENU menu_popup;
  DWORD dwStyle, dwExStyle;

  WNDCLASSEX wclass =
    {sizeof(WNDCLASSEX), 0, harpoon_window_proc, 0, 0, dll_handle, NULL, NULL, NULL, NULL, HARPOON_WINDOW_CLASS, NULL};

  harpoon_exit();

  /*
  This main init function is only called from workrave
  Set exec_filename_workrave/critical TRUE
  */
  exec_filename_workrave = TRUE;
  exec_filename_critical = TRUE;

  notification_class = RegisterClassEx(&wclass);
  if (!notification_class)
    return FALSE;

  if (debug_harpoon)
    // The notification window should be visible and have a menu.
    {
      dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
      dwExStyle = WS_EX_APPWINDOW | WS_EX_STATICEDGE;

      menu = CreateMenu();
      menu_popup = CreatePopupMenu();
      if (menu && menu_popup)
        {
          AppendMenu(menu_popup, MF_STRING, IDM_MENU_SAVE, "&Save");
          AppendMenu(menu_popup, MF_SEPARATOR, 0, 0);
          AppendMenu(menu_popup, MF_STRING | MF_CHECKED, IDM_MENU_MONITOR, "Capture &Debug Messages");
          AppendMenu(menu_popup, MF_SEPARATOR, 0, 0);
          AppendMenu(menu_popup, MF_STRING, IDM_MENU_CLEAR, "&Clear Display");
          AppendMenu(menu_popup, MF_STRING, IDM_MENU_UNHOOK, "&Unhook");
          AppendMenu(menu, MF_POPUP, (UINT_PTR)menu_popup, "&Menu");
        }
      else
        {
          DestroyMenu(menu);
          DestroyMenu(menu_popup);
          menu = menu_popup = NULL;
        }
    }
  else
    {
      dwStyle = WS_OVERLAPPED;
      dwExStyle = WS_EX_TOOLWINDOW;
      menu = menu_popup = NULL;
    }

  notification_window = CreateWindowEx(dwExStyle,
                                       HARPOON_WINDOW_CLASS,
                                       HARPOON_WINDOW_CLASS,
                                       dwStyle,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       NULL,
                                       menu,
                                       dll_handle,
                                       NULL);

  if (!notification_window)
    {
      DestroyMenu(menu);
      UnregisterClass(HARPOON_WINDOW_CLASS, dll_handle);
      notification_class = 0;
      return FALSE;
    }

  if (debug_harpoon)
    // We need an edit control to send debug messages to.
    {
      GetClientRect(notification_window, &rect);

      debug_hwnd = CreateWindow("EDIT",
                                "",
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN,
                                rect.left,
                                rect.top,
                                rect.right,
                                rect.bottom,
                                notification_window,
                                NULL,
                                dll_handle,
                                NULL);

      if (debug_hwnd)
        {
          SendMessage(debug_hwnd, EM_SETLIMITTEXT, 0x7FFFFFFE, 0);
          // Assign debug flag global.
          debug = debug_harpoon;
          debug_send_message(
            "Note: If both WH_MOUSE and WH_MOUSE_LL are hooked, "
            "WH_MOUSE messages are not appended to the debug window. "
            "This is done mainly to avoid overflow.");
          debug_send_message("Initializing...");
        }
      else
        {
          debug = FALSE;
        }
      UpdateWindow(notification_window);
    }

  for (i = 0; i < HARPOON_MAX_UNBLOCKED_APPS; ++i)
    {
      strncpy(critical_file_list[i], imported_critical_file_list[i], 510);
      critical_file_list[i][510] = '\0';
      if (critical_file_list[i][0] != '\0')
        {
          if_debug_send_message("Critical file will not have input blocked:");
          if_debug_send_message(critical_file_list[i]);
        }
    }

  if_debug_send_message("harpoon_init() success");

  initialized = TRUE;

  return TRUE;
}

HARPOON_API void
harpoon_exit(void)
{
  if_debug_send_message("harpoon_exit() called");

  harpoon_unhook();

  initialized = FALSE;
  block_input = FALSE;
  debug = FALSE;
  debug_hwnd = NULL;

  if (notification_window)
    {
      DestroyWindow(notification_window);
      notification_window = NULL;
    }

  if (notification_class)
    {
      UnregisterClass(HARPOON_WINDOW_CLASS, dll_handle);
      notification_class = 0;
    }
}

HARPOON_API void
harpoon_unhook()
{
  if_debug_send_message("harpoon_unhook() called");

  if (msg_hook)
    {
      UnhookWindowsHookEx(msg_hook);
      if_debug_send_message("UnhookWindowsHookEx(msg_hook)");
      msg_hook = NULL;
    }
  if (mouse_hook)
    {
      UnhookWindowsHookEx(mouse_hook);
      if_debug_send_message("UnhookWindowsHookEx(mouse_hook)");
      mouse_hook = NULL;
    }
  if (mouse_ll_hook)
    {
      UnhookWindowsHookEx(mouse_ll_hook);
      if_debug_send_message("UnhookWindowsHookEx(mouse_ll_hook)");
      mouse_ll_hook = NULL;
    }
  if (keyboard_hook)
    {
      UnhookWindowsHookEx(keyboard_hook);
      if_debug_send_message("UnhookWindowsHookEx(keyboard_hook)");
      keyboard_hook = NULL;
    }
  if (keyboard_ll_hook)
    {
      UnhookWindowsHookEx(keyboard_ll_hook);
      if_debug_send_message("UnhookWindowsHookEx(keyboard_ll_hook)");
      keyboard_ll_hook = NULL;
    }
  user_callback = NULL;
}

HARPOON_API BOOL
harpoon_hook(HarpoonHookFunc func, BOOL keyboard_lowlevel, BOOL mouse_lowlevel)
{
  if_debug_send_message("harpoon_hook() called");
  /*
  This hook init function is only called from workrave
  Set exec_filename_workrave/critical TRUE
  */
  exec_filename_workrave = TRUE;
  exec_filename_critical = TRUE;

  if (!notification_window || !func)
    {
      if_debug_send_message("!notification_window || !func");
      if_debug_send_message("harpoon hook initialization failure");
      return FALSE;
    }

  harpoon_unhook();
  user_callback = func;

  if (mouse_lowlevel == TRUE)
    {
      mouse_ll_hook = SetWindowsHookEx(WH_MOUSE_LL, harpoon_mouse_ll_hook, dll_handle, 0);
      if (mouse_ll_hook)
        if_debug_send_message("SetWindowsHookEx: WH_MOUSE_LL (success)");
      else
        if_debug_send_message("SetWindowsHookEx: WH_MOUSE_LL (failure)");
    }

  /*
  WH_MOUSE is always hooked. It's needed to determine which
  applications to block. There is no way to determine the
  destination window/application when using only LL hooks.
  */
  mouse_hook = SetWindowsHookEx(WH_MOUSE, harpoon_mouse_hook, dll_handle, 0);
  if (mouse_hook)
    if_debug_send_message("SetWindowsHookEx: WH_MOUSE (success)");
  else
    if_debug_send_message("SetWindowsHookEx: WH_MOUSE (failure)");

  if (keyboard_lowlevel == TRUE)
    {
      keyboard_ll_hook = SetWindowsHookEx(WH_KEYBOARD_LL, harpoon_keyboard_ll_hook, dll_handle, 0);
      if (keyboard_ll_hook)
        if_debug_send_message("SetWindowsHookEx: WH_KEYBOARD_LL (success)");
      else
        if_debug_send_message("SetWindowsHookEx: WH_KEYBOARD_LL (failure)");
    }

  keyboard_hook = SetWindowsHookEx(WH_KEYBOARD, harpoon_keyboard_hook, dll_handle, 0);
  if (keyboard_hook)
    if_debug_send_message("SetWindowsHookEx: WH_KEYBOARD (success)");
  else
    if_debug_send_message("SetWindowsHookEx: WH_KEYBOARD (failure)");

  if ((!keyboard_hook && !keyboard_ll_hook) || !mouse_hook)
    {
      if_debug_send_message("harpoon_hook() failure");
      return FALSE;
    }
  else
    {
      if_debug_send_message("harpoon_hook() success");
      return TRUE;
    }
}

static void
harpoon_hook_block_only(void)
{
  if_debug_send_message("harpoon_hook_block_only() called");

  /*
    This hook init function is only called from workrave
    Set exec_filename_workrave/critical TRUE
  */
  exec_filename_workrave = TRUE;
  exec_filename_critical = TRUE;

  if (user_callback == NULL)
    {
      harpoon_unhook();

#if defined(_WIN64)
      if (msg_hook == NULL)
        {
          msg_hook = SetWindowsHookEx(WH_GETMESSAGE, harpoon_msg_block_hook, dll_handle, 0);
          if (msg_hook)
            if_debug_send_message("SetWindowsHookEx: WH_GETMESSAGE (success)");
          else
            {
              if_debug_send_message("SetWindowsHookEx: WH_GETMESSAGE (failure)");
              harpoon_exit();
            }
        }
#else
      if (mouse_hook == NULL)
        {
          mouse_hook = SetWindowsHookEx(WH_MOUSE, harpoon_mouse_block_hook, dll_handle, 0);
          if (mouse_hook)
            if_debug_send_message("SetWindowsHookEx: WH_MOUSE (success)");
          else
            {
              if_debug_send_message("SetWindowsHookEx: WH_MOUSE (failure)");
              harpoon_exit();
            }
        }

      if (keyboard_hook == NULL)
        {
          keyboard_hook = SetWindowsHookEx(WH_KEYBOARD, harpoon_keyboard_block_hook, dll_handle, 0);
          if (keyboard_hook)
            if_debug_send_message("SetWindowsHookEx: WH_KEYBOARD (success)");
          else
            {
              if_debug_send_message("SetWindowsHookEx: WH_KEYBOARD (failure)");
              harpoon_exit();
            }
        }
#endif
    }
}

static void
_get_exec_filename()
{
  // keep this code out of dllmain. don't call from dllmain.
  // jay satiro, workrave project, august 2007
  // no _pgmptr. no psapi, for compatibility.
  // GetModuleBaseNameA( GetCurrentProcess(), NULL, lpstr, 255 );
  /*
  GetModuleFileNameA: MSDN does not note:
  -SetLastError code is not set on success (appears <= XP)
  -The maximum number of possible characters returned.
  -On Vista, the copied string is always null terminated:
      if ( bytes_copied == nSize )
          dest_ptr[ nSize - 1 ] = '\0';
      return bytes_copied;
  The last character could be lost if that null is unaccounted for.
  --
  e.g. fullpath = "F:\f\f.exe"
  GetModuleFileNameA( NULL, buffer, 10 ):
  On XP and others, no termination:
  'F',':','\','f','\','f','.','e','x','e'
  On Vista, ERROR_INSUFFICIENT_BUFFER:
  'F',':','\','f','\','f','.','e','x','\0'
  */
  int size;
  DWORD ret;
  unsigned char *p, *buffer;

  for (size = 1024, buffer = NULL; (buffer = realloc(buffer, size + 1)) != NULL; size *= 2)
    // This doubles the buffer until it can hold the filename.
    {
      SetLastError(NO_ERROR);

      ret = GetModuleFileNameA(NULL, (char *)buffer, size);

      if (ret && GetLastError() == NO_ERROR)
        break;

      if (size >= 65536)
        // far enough
        {
          free(buffer);
          buffer = NULL;
          break;
        }
    }

  if (!buffer)
    {
      // Filename can't be ascertained :(
      exec_filename[0] = '\0';
      if_debug_send_message("This application's filename can't be ascertained :(");
      return;
    }

  buffer[ret] = '\0';

  // MS ANSI codepages include DBCS:
  // http://codesnipers.com/?q=node/34

  // Search the path to find where the filename starts:
  if ((p = _mbsrchr(buffer, '\\')) != NULL)
    // Point to first (mb) filename character
    ++p;
  else
    // No path. Probably a Windows Me/98/95 filename
    p = buffer;

  _mbstrncpy_lowercase((char *)exec_filename, (char *)p, 510);
  exec_filename[510] = '\0';

  free(buffer);
}

/**********************************************************************
 * Main
 **********************************************************************/

BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
// don't call init or debug stuff from here
{
  dll_handle = hModule;

  switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
      exec_process_id = GetCurrentProcessId();
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    }

  return TRUE;
}

HARPOON_API char *
_mbstrncpy_lowercase(const char *out, const char *in, int bytes)
// keep this code out of dllmain. don't call from dllmain.
{
  int mb = 0;
  unsigned char *src = (unsigned char *)in;
  unsigned char *dest = (unsigned char *)out;

  if (bytes <= 0 || !in || !out)
    return NULL;

  while ((mb = _mbsnextc(src)) > 0)
    // This makes a lowercase copy of the filename.
    {
      // Point to next (mb) character:
      (mb >> 8) ? (src += 2) : (src += 1);

      mb = _mbctolower(mb);

      *dest++ = (unsigned char)mb;

      if (--bytes == 0)
        break;

      if ((*dest = (unsigned char)(mb >> 8)) != 0)
        {
          ++dest;

          if (--bytes == 0)
            break;
        }
    }

  memset(dest, 0, bytes);

  return (char *)out;
}

/**********************************************************************
 * Debug
 **********************************************************************/

static void
debug_send_message(const char *str)
/*
Here we send a message to the edit control.
debug_send_message( "WH_MOUSE" )
would append something like this:
1:55:05 PM: whatever.exe(123): WH_MOUSE

The passed in str should be no more than 480 bytes long:
max exec_filename len 510
pid/time/formatting chars about 30
buffer len is 1024 - 540 = ~480 max str len
*/
{
  static char str_previous_call[100] = {'\0'};
  static WORD wHour_previous_call = 0;
  static WORD wMinute_previous_call = 0;
  static WORD wSecond_previous_call = 0;

  char buffer[1024];
  char mer[3] = "AM";

  SYSTEMTIME local;
  /*
  SCROLLINFO si =
    {
      sizeof( SCROLLINFO ),
      SIF_POS | SIF_RANGE,
      0, 0, 0, 0, 0
    };
  */

  GetLocalTime(&local);

  if (local.wSecond == wSecond_previous_call && local.wMinute == wMinute_previous_call && local.wHour == wHour_previous_call
      && strncmp(str, str_previous_call, 100) == 0)
    // we only update once per second if the message is the same
    // the first 100 chars of the message are compared to last msg
    return;
  else
    {
      wSecond_previous_call = local.wSecond;
      wMinute_previous_call = local.wMinute;
      wHour_previous_call = local.wHour;
      strncpy(str_previous_call, str, 99);
      str_previous_call[99] = '\0';
    }

  if (local.wHour > 12)
    // Make clock 12 hour, set meridiem identifier to PM
    {
      local.wHour = local.wHour - 12;
      mer[0] = 'P';
    }
  else if (local.wHour == 0) // 12am
    local.wHour = (WORD)12;

  snprintf(buffer,
           1023,
           "%lu:%02lu:%02lu %s:\t%s (%lu):\t %s\r\n",
           (DWORD)local.wHour,
           (DWORD)local.wMinute,
           (DWORD)local.wSecond,
           mer,
           exec_filename,
           exec_process_id,
           str);

  buffer[1023] = '\0';

  // Move caret to the end of current text
  SendMessage(debug_hwnd, EM_SETSEL, 0, -1);
  SendMessage(debug_hwnd, EM_SETSEL, -1, -1);
  // Append at caret position
  SendMessage(debug_hwnd, EM_REPLACESEL, 0, (LPARAM)buffer);

  /*
  GetScrollInfo( debug_hwnd, SB_VERT, &si );
  if( si.nPos >= si.nMax )
  // If the scrollbar position is at the bottom, scroll to caret
      SendMessage( debug_hwnd, EM_SCROLLCARET, 0, 0 );
  */
}

static INLINE void
if_debug_send_message(const char *str)
{
  if (debug)
    debug_send_message(str);
}

static void
debug_process_menu_selection(WORD idm)
{
  DWORD ret;

  switch (idm)
    {
    case IDM_MENU_SAVE:
      if_debug_send_message("IDM_MENU_SAVE");
      debug_save_data();
      break;

    case IDM_MENU_MONITOR:
      if_debug_send_message("IDM_MENU_MONITOR");
      ret = CheckMenuItem(menu, IDM_MENU_MONITOR, MF_UNCHECKED);
      if (ret == MF_UNCHECKED)
        // Menu item was previously unchecked. Toggle to checked.
        {
          CheckMenuItem(menu, IDM_MENU_MONITOR, MF_CHECKED);
          debug = TRUE;
          debug_send_message("-->Message Capture Enabled.");
        }
      else
        // User selected to disable messages.
        {
          debug = FALSE;
          debug_send_message("-->Message Capture Disabled.");
        }
      break;

    case IDM_MENU_CLEAR:
      SetWindowText(debug_hwnd, "");
      if_debug_send_message("IDM_MENU_CLEAR");
      break;

    case IDM_MENU_UNHOOK:
      harpoon_unhook();
      if_debug_send_message("IDM_MENU_UNHOOK");
      break;
    }
  return;
}

static void
debug_save_data()
// This is called if the user opts to save the messages to a file.
{
  char *buffer;
  char filename[MAX_PATH] = {'\0'};
  int unload = FALSE;
  HANDLE handle;
  HMODULE hmm;
  DWORD ret, text_length;

  OPENFILENAME ofn = {
#if defined(WINVER) && (_WIN32_WINNT >= 0x0500)
    OPENFILENAME_SIZE_VERSION_400,
#else
    sizeof(OPENFILENAME),
#endif
    debug_hwnd,                              // hwndOwner
    NULL,                                    // hInstance
    "All Files\0*.*\0\0",                    // lpstrFilter
    NULL,                                    // lpstrCustomFilter
    0,                                       // nMaxCustFilter
    1,                                       // nFilterIndex
    filename,                                // lpstrFile
    MAX_PATH,                                // nMaxFile
    NULL,                                    // lpstrFileTitle
    0,                                       // nMaxFileTitle
    NULL,                                    // lpstrInitialDir
    NULL,                                    // lpstrTitle
    OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, // Flags
    0,                                       // nFileOffset
    0,                                       // nFileExtension
    "txt",                                   // lpstrDefExt
    0,                                       // lCustData
    NULL,                                    // lpfnHook
    NULL                                     // lpTemplateName
#if defined(WINVER) && (_WIN32_WINNT >= 0x0500)
    ,
    NULL,
    0,
    0
#endif
  };

  BOOL(WINAPI * GetSaveFileNameA)(OPENFILENAME *);

  if ((hmm = GetModuleHandleA("comdlg32.dll")) == NULL)
    {
      if ((hmm = LoadLibraryA("comdlg32.dll")) == NULL)
        {
          debug_send_message("-->Data not saved. LoadLibrary() failed");
          return;
        }
      unload = TRUE;
    }

  GetSaveFileNameA = (BOOL(WINAPI *)(OPENFILENAME *))GetProcAddress(hmm, "GetSaveFileNameA");

  if (GetSaveFileNameA == NULL)
    {
      debug_send_message("-->Data not saved. GetProcAddress() failed");
      if (unload)
        FreeLibrary(hmm);
      return;
    }

  if ((*GetSaveFileNameA)(&ofn) == 0)
    {
      debug_send_message("-->Data not saved. GetSaveFileName() failed");
      if (unload)
        FreeLibrary(hmm);
      return;
    }

  if (unload)
    FreeLibrary(hmm);

  handle = CreateFile(filename,              // lpFileName
                      GENERIC_WRITE,         // dwDesiredAccess
                      0,                     // dwShareMode
                      NULL,                  // lpSecurityAttributes
                      CREATE_ALWAYS,         // dwCreationDisposition
                      FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
                      NULL                   // hTemplateFile
  );

  if (handle == INVALID_HANDLE_VALUE)
    {
      debug_send_message("-->Data not saved. CreateFile() failed");
      return;
    }

  text_length = (DWORD)GetWindowTextLength(debug_hwnd);
  if (!text_length)
    {
      debug_send_message("-->Data not saved. GetWindowTextLength() failed");
      CloseHandle(handle);
      return;
    }

  if (!(buffer = malloc(text_length + 1)))
    {
      debug_send_message("-->Data not saved. malloc() failed");
      CloseHandle(handle);
      return;
    }

  ret = (DWORD)GetWindowText(debug_hwnd, buffer, (int)text_length + 1);
  if (ret != text_length)
    {
      debug_send_message("-->Data not saved. GetWindowText() failed");
      CloseHandle(handle);
      free(buffer);
      return;
    }

  buffer[text_length] = '\0';

  WriteFile(handle,      // hFile
            buffer,      // lpBuffer
            text_length, // nNumberOfBytesToWrite
            &ret,        // lpNumberOfBytesWritten (always first set to 0 by WriteFile)
            NULL         // lpOverlapped
  );

  if (ret != text_length)
    debug_send_message("-->Data possibly not saved. WriteFile() failed");

  CloseHandle(handle);
  free(buffer);
  return;
}
