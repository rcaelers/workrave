#include "W32Compat.hh"

BOOL W32Compat::initialized = false;
BOOL W32Compat::is_w95 = false;
W32Compat::ENUMDISPLAYMONITORSPROC W32Compat::enum_display_monitors_proc = NULL;
W32Compat::GETMONITORINFOPROC W32Compat::get_monitor_info_proc = NULL;
W32Compat::MONITORFROMPOINTPROC W32Compat::monitor_from_point_proc = NULL;

BOOL
W32Compat::IsWindows95()
{
  init();
  return is_w95;
}


BOOL
W32Compat::EnumDisplayMonitors(HDC hdc,LPCRECT rect,
                               MONITORENUMPROC proc,
                               LPARAM lparam)
{
  init();
  if (enum_display_monitors_proc != NULL)
    {
      return (*enum_display_monitors_proc)(hdc, rect, proc, lparam);
    }
  return FALSE;
}

BOOL
W32Compat::GetMonitorInfo(HMONITOR monitor, LPMONITORINFO info)
{
  init();
  if (get_monitor_info_proc != NULL)
    {
      return (*get_monitor_info_proc)(monitor, info);
    }
  return FALSE;
}

HMONITOR
W32Compat::MonitorFromPoint(POINT pt, DWORD dwFlags)
{
  init();
  if (monitor_from_point_proc != NULL)
    {
      return (*monitor_from_point_proc)(pt, dwFlags);
    }
  return NULL;
}

void
W32Compat::init()
{
  if (initialized)
    return;
  
  HINSTANCE user_lib = LoadLibrary("user32.dll");
  if (user_lib)
    {
      enum_display_monitors_proc = (ENUMDISPLAYMONITORSPROC) GetProcAddress(user_lib,"EnumDisplayMonitors");
      get_monitor_info_proc = (GETMONITORINFOPROC) GetProcAddress(user_lib, "GetMonitorInfoA");
      monitor_from_point_proc = (MONITORFROMPOINTPROC) GetProcAddress(user_lib, "MonitorFromPoint");
    }


  DWORD dwVersion = GetVersion();
  DWORD major =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
  DWORD minor =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
  is_w95 = (major == 4 && minor == 0);
  

  initialized = TRUE;
}


