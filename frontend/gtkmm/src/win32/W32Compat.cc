#include "debug.hh"

#include "W32Compat.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

using namespace workrave;

bool W32Compat::run_once = true; //initialization
bool W32Compat::force_focus = false;
bool W32Compat::ime_magic = false;
bool W32Compat::reset_window_always = false;
bool W32Compat::reset_window_never = false;

W32Compat::ENUMDISPLAYMONITORSPROC W32Compat::enum_display_monitors_proc = NULL;
W32Compat::GETMONITORINFOPROC W32Compat::get_monitor_info_proc = NULL;
W32Compat::MONITORFROMPOINTPROC W32Compat::monitor_from_point_proc = NULL;
W32Compat::SWITCHTOTHISWINDOWPROC W32Compat::switch_to_this_window_proc = NULL;


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

VOID
W32Compat::SwitchToThisWindow( HWND hwnd, BOOL emulate_alt_tab )
{
  init();
  if ( switch_to_this_window_proc != NULL )
    {
      ( *switch_to_this_window_proc )( hwnd, emulate_alt_tab );
    }
  return;
}

void
W32Compat::init_once()
{
	run_once = false;

	HMODULE user_lib = GetModuleHandleA( "user32.dll" );
	if( user_lib )
	{
		enum_display_monitors_proc = (ENUMDISPLAYMONITORSPROC) GetProcAddress(user_lib,"EnumDisplayMonitors");
		get_monitor_info_proc = (GETMONITORINFOPROC) GetProcAddress(user_lib, "GetMonitorInfoA");
		monitor_from_point_proc = (MONITORFROMPOINTPROC) GetProcAddress(user_lib, "MonitorFromPoint");
		switch_to_this_window_proc = (SWITCHTOTHISWINDOWPROC) GetProcAddress(user_lib, "SwitchToThisWindow");
	}

	// Should SetWindowOnTop() call ForceWindowFocus() ?
	if( !CoreFactory::get_configurator()->get_value( "advanced/force_focus", force_focus ) )
	{
		force_focus = false;
	}

	// Should SetWindowOnTop() call IMEWindowMagic() ?
	if( !CoreFactory::get_configurator()->get_value( "advanced/ime_magic", ime_magic ) )
	{
		ime_magic = false;
	}

	// As of writing SetWindowOnTop() always calls ResetWindow()
	// ResetWindow() determines whether to "reset" when both
	// reset_window_always and reset_window_never are false.
	//
	// If reset_window_always is true, and if ResetWindow() is continually
	// passed the same hwnd, hwnd will flicker as a result of the continual
	// z-order position changes / resetting.
	if( !CoreFactory::get_configurator()->get_value( "advanced/reset_window_always", reset_window_always ) )
	{
		reset_window_always = false;
	}
	// ResetWindow() will always abort when reset_window_never is true.
	if( !CoreFactory::get_configurator()->get_value( "advanced/reset_window_never", reset_window_never ) )
	{
		reset_window_never = false;
	}

}

bool
W32Compat::get_force_focus_value()
{
	init();

	return force_focus;
}


void
W32Compat::SetWindowOnTop( HWND hwnd, BOOL topmost )
{
	init();

	SetWindowPos( hwnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	ResetWindow( hwnd, (bool)topmost );

	if( ime_magic && topmost )
	{
		IMEWindowMagic( hwnd );
	}

	if( force_focus && topmost )
	{
		ForceWindowFocus( hwnd );
	}

	return;
}


// ForceWindowFocus() code was written in 2007 for Vista UIPI
// troubleshooting. It appears to no longer be useful, but I've
// cleaned it up should it possibly serve some other use.
// In 2008 the hack to undo admin focus stopped working.
// if all else fails request user enable advanced/force_focus = "1"
bool
W32Compat::ForceWindowFocus( HWND hwnd )
{
	init();

	if( !IsWindow( hwnd ) )
		return false;

	/* FYI:
	* GetForegroundWindow returns NULL if there is no window in focus.
	*/

	if( GetForegroundWindow() != hwnd )
	{
		// SwitchToThisWindow() should activate and set focus
		// set 'TRUE' to emulate alt+tab ordering. no return value.
		SwitchToThisWindow( hwnd, TRUE );
	}


	if( GetForegroundWindow() != hwnd )
	/*
	If the foreground window is still not our window, we can try
	to AttachThreadInput on the current foreground window as a
	last resort. This method has been in use since Windows 98.
	.
	This method doesn't work on foreground console windows,
	or windows of a higher integrity level (Vista).
	.
	Attaching input can cause deadlock.
	Please do not use this as inspiration for your own code.
	.
	This block is a last resort for force_focus troubleshooting only.
	*/
	{
		DWORD dThisThread, dForeThread, dForePID;

		dThisThread = GetCurrentThreadId();
		dForeThread = GetWindowThreadProcessId( GetForegroundWindow(), &dForePID );

		if( AttachThreadInput( dThisThread, dForeThread, TRUE ) )
		{
			BringWindowToTop( hwnd );
			SetForegroundWindow( hwnd );

			AttachThreadInput( dThisThread, dForeThread, FALSE );
		}
	}

	return GetForegroundWindow() == hwnd;
}


// Bug 587 -  Vista: Workrave not modal / coming to front
// http://issues.workrave.org/cgi-bin/bugzilla/show_bug.cgi?id=587
// There is an issue with IME and window z-ordering.
//
// hwnd == window to "reset" in z-order
// topmost == true if window should be topmost, false otherwise
void
W32Compat::ResetWindow( HWND hwnd, bool topmost )
{
	init();

	if( !IsWindow( hwnd ) || reset_window_never )
		return;

	const bool DEBUG = false;
	bool reset = false;
	DWORD gwl_exstyle = 0;
	DWORD valid_exstyle_diff = 0;

	WINDOWINFO gwi;
	ZeroMemory( &gwi, sizeof( gwi ) );
	gwi.cbSize = sizeof( WINDOWINFO );


	SetLastError( 0 );
	gwl_exstyle = (DWORD)GetWindowLong( hwnd, GWL_EXSTYLE );
	if( !GetLastError() )
	{
		// if desired and actual topmost style differ, plan to reset
		if( topmost != ( gwl_exstyle & WS_EX_TOPMOST ? true : false ) )
			reset = true;
	}

	SetLastError( 0 );
	GetWindowInfo( hwnd, &gwi );
	if( !GetLastError() )
	{
		// if desired and actual topmost style differ, plan to reset
		if( topmost != ( gwi.dwExStyle & WS_EX_TOPMOST ? true : false ) )
			reset = true;
	}

	// GetWindowInfo() and GetWindowLong() extended style info can differ.
	// Compare the two results but filter valid values only.
	valid_exstyle_diff = ( gwl_exstyle ^ gwi.dwExStyle ) & ~0xF1A08802;
	if( valid_exstyle_diff || DEBUG )
	{
		// if the extended style info differs, plan to reset.
		// e.g. gwl returned ws_ex_toolwindow but gwi didn't
		reset = true;

#ifdef BREAKAGE
		// attempt to sync differences:
		DWORD swl_exstyle = ( valid_exstyle_diff | gwl_exstyle ) & ~0xF1A08802;

		if( ( swl_exstyle & WS_EX_APPWINDOW ) && ( swl_exstyle & WS_EX_TOOLWINDOW ) )
		// this hasn't happened and shouldn't happen, but i suppose it could.
		// if both styles are set change to appwindow only.
		// why not toolwindow only? well, why are they both set in the first place?
		// in this case it's better to make hwnd visible on the taskbar.
		{
			swl_exstyle &= ~WS_EX_TOOLWINDOW;
		}

		ShowWindow( hwnd, SW_HIDE );
		SetWindowLong( hwnd, GWL_EXSTYLE, (LONG)swl_exstyle );
		ShowWindow( hwnd, SW_SHOWNA );
#endif
	}

	// "reset" window position in z-order.
	// if the window is supposed to be topmost but is really not:
	// set HWND_NOTOPMOST followed by HWND_TOPMOST
	// the above sequence is key: review test results in 587#c17
	//
	// if the window is not supposed to be topmost but is, reverse:
	// set HWND_TOPMOST followed by HWND_NOTOPMOST
	// the reverse is currently unproven.
	// i don't know of any problems removing the topmost style.
	if( IsWindow( hwnd ) && ( reset || reset_window_always ) )
	{
		SetWindowPos( hwnd, !topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
		SetWindowPos( hwnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
	}

	return;
}


// Bug 587 -  Vista: Workrave not modal / coming to front
// http://issues.workrave.org/cgi-bin/bugzilla/show_bug.cgi?id=587
// There is an issue with IME and window z-ordering.
//
// ResetWindow() tests sufficient. This code is for troubleshooting.
// if all else fails request user enable advanced/ime_magic = "1"
void
W32Compat::IMEWindowMagic( HWND hwnd )
{
	TRACE_ENTER( "W32Compat::IMEWindowMagic" );

	init();

	if( !IsWindow( hwnd ) )
		return;

	// This message works to make hwnd topmost without activation or focus.
	// I found it by watching window messages. I don't know its intended use.
	SendMessage( hwnd, 0x287, 0x17/*0x18*/, (LPARAM)hwnd );

	SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOOWNERZORDER |
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	TRACE_EXIT();
}
