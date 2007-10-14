#include "W32Compat.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

using namespace workrave;

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


BOOL W32Compat::SetWindowOnTop( HWND hwnd, BOOL top )
// jay satiro, workrave project, may 2007
// redistribute under GNU terms.
{
  static bool run_once = true;
  static bool force_focus = false;
  static VOID ( WINAPI *SwitchToThisWindow ) ( HWND, BOOL ) = NULL;
  if( run_once )
    {
      SwitchToThisWindow = ( VOID ( WINAPI * ) ( HWND, BOOL ) )
        GetProcAddress( GetModuleHandleA( "user32.dll" ), "SwitchToThisWindow" );
  
      if( !CoreFactory::get_configurator()->get_value( "advanced/force_focus", force_focus ) )
        force_focus = false;
  
      run_once = false;
    }
  
  
  BOOL rv;
  HWND hFore;
  
  rv = SetWindowPos( hwnd, top ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
  /*
    if( !rv )
    rv = BringWindowToTop( hwnd );
  */
  hFore = GetForegroundWindow();
  /* FYI:
     GetForegroundWindow returns NULL if there is no window in focus.
  */
  
  if( !force_focus || !top || hFore == hwnd )
    {
      /*
        Return if we don't need to force the focus on a topmost window.
      */
      return rv;
    }
  
  
  /*
    force_focus is here to tackle some compatibility issues I have
    not yet figured out. As best I can tell, there is an issue
    with Vista's UIPI and the window ordering.
    .
    Remarkably, this isn't easy to debug, as there is no reliable
    way to reproduce. And even if there is, when some API calls
    fail in Vista due to UIPI issues, they still return SUCCESS.
  */
  
  /*
    If the current foreground window in Vista, 'hFore', is of a
    higher integrity level, sending input to it can kill its focus.
    *This makes certain assumptions, too many to cover in comments.
    .
    This is a cheap hack but it works. Unfortunately Windows counts
    this as an input event, so activity monitoring during breaks
    must be disabled, or else the timer will freeze every time focus
    has to be forced.
  */
  keybd_event( 0, 0, 0, 0 );
  
  // Now we should be able to switch in Win2000 or better:
  if( SwitchToThisWindow )
    // set 'TRUE' to emulate alt+tab ordering. no return value.
    ( *SwitchToThisWindow )( hwnd, TRUE );
  
  
  hFore = GetForegroundWindow();
  
  if( hFore != hwnd )
    /*
      If the foreground window is still not our window, we can try
      to AttachThreadInput on the current foreground window as a
      last resort. This method has been in use since Windows 98.
      .
      I've found this method has some problems, like it doesn't
      work on foreground console windows, or windows with a higher
      integrity level (Vista).
    */
    {
      DWORD dThisThread, dForeThread, dForePID;
    
      dThisThread = GetCurrentThreadId();
      dForeThread = GetWindowThreadProcessId( hFore, &dForePID );
    
      AttachThreadInput( dThisThread, dForeThread, TRUE );
    
      BringWindowToTop( hwnd );
      SetForegroundWindow( hwnd );
    
      AttachThreadInput( dThisThread, dForeThread, FALSE );
    }
  // regardless we return the original setwindowpos return value:
  return rv;
}
