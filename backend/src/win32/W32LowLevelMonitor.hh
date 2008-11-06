// W32LowLevelMonitor.cc --- ActivityMonitor for W32
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef W32LOWLEVELMONITOR_HH
#define W32LOWLEVELMONITOR_HH

#include "InputMonitor.hh"

class W32LowLevelMonitor :
  public InputMonitor
{
public:
  W32LowLevelMonitor();
  virtual ~W32LowLevelMonitor();
  bool init(); 
  void terminate();
  static W32LowLevelMonitor *singleton;
  
protected:
  static DWORD WINAPI thread_Dispatch( LPVOID );
  static DWORD WINAPI thread_Callback( LPVOID );
  
private:
  
  struct thread_struct
    {
      volatile bool active;
      DWORD id;
      HANDLE handle;
      const char *name;
      thread_struct()
        {
          active = false;
          id = 0;
          handle = NULL;
          name = NULL;
        }
    };
  static thread_struct *dispatch, *callback;
  
  bool wait_for_thread_queue( thread_struct * );
  void terminate_thread( thread_struct * );
  void wait_for_thread_to_exit( thread_struct * );
  void unhook();
  
  DWORD dispatch_thread();
  DWORD time_critical_callback_thread();
  
  static LRESULT CALLBACK k_hook_callback( int, WPARAM, LPARAM );
  static LRESULT CALLBACK m_hook_callback( int, WPARAM, LPARAM );
  
  static volatile HHOOK k_hook;
  static volatile HHOOK m_hook;
  
  bool check_api();
  static HMODULE process_handle;
  static BOOL ( WINAPI *GetMessageW ) ( LPMSG, HWND, UINT, UINT );
  static BOOL ( WINAPI *PeekMessageW ) ( LPMSG, HWND, UINT, UINT, UINT );
  static BOOL ( WINAPI *PostThreadMessageW ) ( DWORD, UINT, WPARAM, LPARAM );
  static HHOOK ( WINAPI *SetWindowsHookExW ) ( int, HOOKPROC, HINSTANCE, DWORD );
  static BOOL ( WINAPI *SwitchToThread ) ( void );
  
};

#ifndef PLATFORM_OS_WIN32_NATIVE
#define WM_XBUTTONDOWN 523
#define WM_XBUTTONUP 524
#define WM_XBUTTONDBLCLK 525
#define WM_MOUSEHWHEEL 526
#endif

#define LLKHF_INJECTED 0x00000010
#define LLMHF_INJECTED 0x00000001

typedef struct {
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
    DWORD time;
    ULONG_PTR dwExtraInfo;
} _KBDLLHOOKSTRUCT;

typedef struct {
    POINT pt;
    DWORD mouseData;
    DWORD flags;
    DWORD time;
    ULONG_PTR dwExtraInfo;
} _MSLLHOOKSTRUCT;

#endif // W32LOWLEVELMONITOR_HH
