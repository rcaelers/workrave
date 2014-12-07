/*
 * crashlog.c
 *
 * Copyright (C) 2003, 2004, 2005, 2007, 2011 Rob Caelers <robc@krandor.nl>
 * Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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
 * Based on Dr. Mingw. and OpenTTD
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "crashlog.h"
#include "harpoon.h"

#include <fcntl.h>
#include <io.h>

#ifndef _O_APPEND
#define _O_APPEND       0x0008
#endif
#ifndef _O_TEXT
#define _O_TEXT         0x4000
#endif

#ifdef PLATFORM_OS_WIN32_NATIVE
#define snprintf _snprintf
#define snwprintf _snwprintf
#endif

static void unwind_stack(FILE *log, HANDLE process, PCONTEXT context);
static void dump_registers(FILE *log, PCONTEXT context);
static void dump_registry(FILE *log, HKEY key, char *name);
/* static void print_module_list(FILE *log); */

static
DWORD GetModuleBase(DWORD dwAddress)
{
  MEMORY_BASIC_INFORMATION Buffer;

  return VirtualQuery((LPCVOID) dwAddress, &Buffer, sizeof(Buffer)) ? (DWORD) Buffer.AllocationBase : 0;
}

#ifndef PLATFORM_OS_WIN32_NATIVE
static EXCEPTION_DISPOSITION __cdecl
double_exception_handler(struct _EXCEPTION_RECORD *exception_record,
                         void *establisher_frame,
                         struct _CONTEXT *context_record,
                         void *dispatcher_context)
{
  (void) exception_record;
  (void) establisher_frame;
  (void) context_record;
  (void) dispatcher_context;

  MessageBox(NULL,
             (LPCSTR)"Workrave has unexpectedly crashed and failed to create a crash "
             "log. This is serious. Please report this to crashes@workrave.org or "
             "file a bugreport at: http://issues.workrave.org/. " ,
			 (LPCSTR)"Double exception", MB_OK);

  exit(1);
}
#endif

EXCEPTION_DISPOSITION __cdecl
exception_handler(struct _EXCEPTION_RECORD *exception_record,
                  void *establisher_frame,
                  struct _CONTEXT *context_record,
                  void *dispatcher_context)
{
  char crash_log_name[MAX_PATH];
  char crash_text[1024];
  TCHAR szModule[MAX_PATH];
  HMODULE hModule;
/*
 Modified for Unicode >= WinNT. No UnicoWS check for Me/98/95.
 jay satiro, workrave project, july 2007
*/
  WCHAR env_var[ 20 ] = { '\0', };
  WCHAR crashlog[] = L"\\workrave-crashlog.txt";
  WCHAR *wbuffer = NULL;
  WCHAR *p_wbuffer = NULL;  FILE *log;
  DWORD ( WINAPI *GetEnvironmentVariableW ) ( LPCWSTR, LPWSTR, DWORD );
  HANDLE ( WINAPI *CreateFileW ) ( LPCWSTR, DWORD, DWORD,
  LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE );
  SYSTEMTIME SystemTime;

  (void) establisher_frame;
  (void) dispatcher_context;

#ifdef PLATFORM_OS_WIN32_NATIVE
  // FIXME: win32
#else
  __try1(double_exception_handler);
#endif

  harpoon_unblock_input();




  GetEnvironmentVariableW = ( DWORD ( WINAPI * ) ( LPCWSTR, LPWSTR, DWORD ) )
    GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "GetEnvironmentVariableW" );
  CreateFileW = ( HANDLE ( WINAPI * ) ( LPCWSTR, DWORD, DWORD,
    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE ) )
      GetProcAddress( GetModuleHandleA( "kernel32.dll" ), "CreateFileW" );

  if( GetEnvironmentVariableW && CreateFileW )
  // >= WinNT
    {
      HANDLE handle;
      DWORD bufsize, ret;
      int fd = 0;

      wcsncpy( env_var, L"APPDATA", 19 );
      env_var[ 19 ] = '\0';
      bufsize = ( *GetEnvironmentVariableW ) ( env_var, NULL, 0 );
      // bufsize is size in wide chars, including null

      if( !bufsize )
      // If %appdata% is unsuitable, try temp:
      {
          wcsncpy( env_var, L"TEMP", 19 );
          env_var[ 19 ] = '\0';
          bufsize = ( *GetEnvironmentVariableW ) ( env_var, NULL, 0 );
      }

      ret = 0;
      wbuffer = NULL;

      if( bufsize )
        {
          // We will need room for \\?\ so add 4
          if( (wbuffer = (WCHAR *)calloc( 4 + bufsize + wcslen( crashlog ), sizeof( WCHAR ) ) ) != NULL)
            {
              wcscpy( wbuffer, L"\\\\?\\" );
              p_wbuffer = wbuffer + 4;
              ret = ( *GetEnvironmentVariableW ) ( env_var, p_wbuffer, bufsize );
            }
        }

      if( !ret )
      // Environment unsuitable, notify & terminate.
        {
          free( wbuffer );
          snprintf(crash_text, 1023,
            "Workrave has unexpectedly crashed. The environment is "
            "unsuitable to create a crash log. Please file a bug report:\n"
            "http://issues.workrave.org/\n"
            "Thanks.");
          MessageBoxA( NULL, crash_text, "Exception", MB_OK );

#ifdef PLATFORM_OS_WIN32_NATIVE
  // FIXME: win32
#else
          __except1;
#endif
          exit( 1 );
        }

      //last wchar
      p_wbuffer = wbuffer + wcslen(wbuffer) - 1;

      while( *p_wbuffer == L'\\' )
      // remove trailing slashes
        {
          *p_wbuffer-- = L'\0';
        }

      // append filename to end of string
      wcscpy( ++p_wbuffer, crashlog );


      // compare first wchar of returned environment string
      if( wbuffer[ 4 ] == L'\\' )
      /*
      If possible network path, don't include literal \\?\
      \\?\\\1.2.3.4\workrave-crashlog.txt should be
      \\1.2.3.4\workrave-crashlog.txt
      */
          p_wbuffer = wbuffer + 4;
      else
      // Point to start of wbuffer:
          p_wbuffer = wbuffer;


      handle = ( *CreateFileW ) (
          p_wbuffer,
          GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_READ,
          NULL,
          CREATE_ALWAYS,
          FILE_ATTRIBUTE_NORMAL,
          NULL
        );

      fd = _open_osfhandle( (intptr_t) handle, _O_APPEND | _O_TEXT );
      log = _fdopen( fd, "w" );
    }
  else  // if( GetVersion() & (DWORD) 0x80000000 )
  // Windows Me/98/95
    {
      char *s = NULL;
      GetModuleFileName(GetModuleHandle(NULL), crash_log_name, sizeof(crash_log_name));
      // crash_log_name == c:\program files\workrave\lib\workrave.exe
      s = strrchr(crash_log_name, '\\');
      assert (s);
      *s = '\0';
      // crash_log_name == c:\program files\workrave\lib
      s = strrchr(crash_log_name, '\\');
      assert (s);
      *s = '\0';
      // crash_log_name == c:\program files\workrave
      strcat(crash_log_name, "\\workrave-crashlog.txt");

      log = fopen(crash_log_name, "w");
    }

    if( log == NULL )
      // workrave-crashlog.txt wasn't created.
      {
        snprintf(crash_text, 1023,
          "Workrave has unexpectedly crashed. An attempt to create "
          "a crashlog has failed. Please file a bug report:\n"
          "http://issues.workrave.org/\n"
          "Thanks.");
        MessageBoxA( NULL, crash_text, "Exception", MB_OK );
#ifdef PLATFORM_OS_WIN32_NATIVE
  // FIXME: win32
#else
        __except1;
#endif
        exit( 1 );
      }


  GetLocalTime(&SystemTime);
  fprintf(log, "Crash log created on %02d/%02d/%04d at %02d:%02d:%02d.\n\n",
          SystemTime.wDay,
          SystemTime.wMonth,
          SystemTime.wYear,
          SystemTime.wHour,
          SystemTime.wMinute,
          SystemTime.wSecond);

  fprintf(log, "version = %s\n", PACKAGE_VERSION);
  fprintf(log, "compile date = %s\n", __DATE__);
  fprintf(log, "compile time = %s\n", __TIME__);
  fprintf(log, "features = "
#ifdef HAVE_DISTRIBUTION
          "DISTRIBUTION "
#endif
#ifdef HAVE_EXERCISES
          "EXERCISES "
#endif
#ifdef HAVE_GCONF
          "GCONF?? "
#endif
#ifdef HAVE_GDOME
          "GDOME "
#endif
#ifdef HAVE_GNET
          "GNET "
#endif
#ifdef HAVE_GNET2
          "GNET2 "
#endif
#ifdef HAVE_XRECORD
          "XRECORD?? "
#endif
#ifndef NDEBUG
          "DEBUG "
#endif
          "\n"
          );

// write locale info:
  char *buffer = NULL;
  int bufsize =
      GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, buffer, 0);

  if( bufsize )
      buffer = (char *)calloc( bufsize + 1, 1 );

  if( buffer )
    {
      GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, buffer, bufsize);
      buffer[ bufsize ] = '\0';
      fprintf( log, "locale = %s\n", buffer );
      free( buffer );
    }

  fprintf(log, "\n\n");
  fprintf(log, "code = %x\n", (int) exception_record->ExceptionCode);
  fprintf(log, "flags = %x\n", (int) exception_record->ExceptionFlags);
  fprintf(log, "address = %x\n", (int) exception_record->ExceptionAddress);
  fprintf(log, "params = %d\n", (int) exception_record->NumberParameters);

  fprintf(log, "%s caused ",  GetModuleFileName(NULL, szModule, MAX_PATH) ? szModule : "Application");
  switch (exception_record->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
      fprintf(log, "an Access Violation");
      break;

    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      fprintf(log, "an Array Bound Exceeded");
      break;

    case EXCEPTION_BREAKPOINT:
      fprintf(log, "a Breakpoint");
      break;

    case EXCEPTION_DATATYPE_MISALIGNMENT:
      fprintf(log, "a Datatype Misalignment");
      break;

    case EXCEPTION_FLT_DENORMAL_OPERAND:
      fprintf(log, "a Float Denormal Operand");
      break;

    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      fprintf(log, "a Float Divide By Zero");
      break;

    case EXCEPTION_FLT_INEXACT_RESULT:
      fprintf(log, "a Float Inexact Result");
      break;

    case EXCEPTION_FLT_INVALID_OPERATION:
      fprintf(log, "a Float Invalid Operation");
      break;

    case EXCEPTION_FLT_OVERFLOW:
      fprintf(log, "a Float Overflow");
      break;

    case EXCEPTION_FLT_STACK_CHECK:
      fprintf(log, "a Float Stack Check");
      break;

    case EXCEPTION_FLT_UNDERFLOW:
      fprintf(log, "a Float Underflow");
      break;

    case EXCEPTION_GUARD_PAGE:
      fprintf(log, "a Guard Page");
      break;

    case EXCEPTION_ILLEGAL_INSTRUCTION:
      fprintf(log, "an Illegal Instruction");
      break;

    case EXCEPTION_IN_PAGE_ERROR:
      fprintf(log, "an In Page Error");
      break;

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      fprintf(log, "an Integer Divide By Zero");
      break;

    case EXCEPTION_INT_OVERFLOW:
      fprintf(log, "an Integer Overflow");
      break;

    case EXCEPTION_INVALID_DISPOSITION:
      fprintf(log, "an Invalid Disposition");
      break;

    case EXCEPTION_INVALID_HANDLE:
      fprintf(log, "an Invalid Handle");
      break;

    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      fprintf(log, "a Noncontinuable Exception");
      break;

    case EXCEPTION_PRIV_INSTRUCTION:
      fprintf(log, "a Privileged Instruction");
      break;

    case EXCEPTION_SINGLE_STEP:
      fprintf(log, "a Single Step");
      break;

    case EXCEPTION_STACK_OVERFLOW:
      fprintf(log, "a Stack Overflow");
      break;

    case DBG_CONTROL_C:
      fprintf(log, "a Control+C");
      break;

    case DBG_CONTROL_BREAK:
      fprintf(log, "a Control+Break");
      break;

    case DBG_TERMINATE_THREAD:
      fprintf(log, "a Terminate Thread");
      break;

    case DBG_TERMINATE_PROCESS:
      fprintf(log, "a Terminate Process");
      break;

    case RPC_S_UNKNOWN_IF:
      fprintf(log, "an Unknown Interface");
      break;

    case RPC_S_SERVER_UNAVAILABLE:
      fprintf(log, "a Server Unavailable");
      break;

    default:
      fprintf(log, "an Unknown [0x%lX] Exception", exception_record->ExceptionCode);
      break;
    }

  fprintf(log, " at location %08x", (int) exception_record->ExceptionAddress);
  hModule = (HMODULE) GetModuleBase((DWORD) exception_record->ExceptionAddress);
  if ((hModule != NULL && GetModuleFileName(hModule, szModule, sizeof(szModule))))
    fprintf(log, " in module %s", szModule);

  // If the exception was an access violation, print out some additional information, to the error log and the debugger.
  if(exception_record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && exception_record->NumberParameters >= 2)
    fprintf(log, " %s location %08x\n\n", exception_record->ExceptionInformation[0] ? "writing to" : "reading from", exception_record->ExceptionInformation[1]);

  DWORD pid = GetCurrentProcessId();
  HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);

  dump_registers(log, context_record);
  unwind_stack(log, process, context_record);

  print_module_list(log);

  fprintf(log, "\nRegistry dump:\n\n");
  dump_registry(log, HKEY_CURRENT_USER, "Software\\Workrave");

  fclose(log);

  if( GetEnvironmentVariableW && CreateFileW )
  // >= WinNT
    {
      WCHAR *one =
          L"Workrave has unexpectedly crashed. A crash log has been saved to:\n";

      WCHAR *two =
          L"\nPlease file a bug report: http://issues.workrave.org/\n"
          L"Thanks.";

      WCHAR *nomem =
          L"Workrave is out of memory!";

      int size = wcslen( one ) + wcslen( p_wbuffer ) + wcslen( two ) + 1;

      WCHAR *message = (WCHAR *)calloc( size, sizeof( WCHAR ) );
      if( !message )
      // Low memory...
        {
          // % + % + null = 3 extra
          size = wcslen( one ) + wcslen( env_var ) + wcslen( crashlog ) + wcslen( two ) + 3;
          message = (WCHAR *)calloc( size, sizeof( WCHAR ) );
          if( message )
            {
              snwprintf( message, size - 1, L"%ws%%%ws%%%ws%ws",
                  one, env_var, crashlog, two );
              message[ size - 1 ] = L'\0';
            }
           else
           // No memory...
              message = nomem;
        }
      else
      // A buffer was allocated with enough memory to hold p_wbuffer
        {
          snwprintf( message, size - 1, L"%ws%ws%ws", one, p_wbuffer, two );
          message[ size - 1 ] = L'\0';
        }

      MessageBoxW( NULL, message, L"Exception", MB_OK );
    }
  else
    {
      snprintf(crash_text, 1023,
        "Workrave has unexpectedly crashed. A crash log has been saved to "
        "%s. Please mail this file to crashes@workrave.org or "
        "file a bugreport at: http://issues.workrave.org/. "
        "Thanks.", crash_log_name);
      MessageBoxA(NULL, crash_text, "Exception", MB_OK);
    }

#ifdef PLATFORM_OS_WIN32_NATIVE
  // FIXME: win32
#else
  __except1;
#endif

  exit(1);
}


static
BOOL WINAPI stack_walk(HANDLE process, LPSTACKFRAME stack_frame, PCONTEXT context_record)
{
  if (!stack_frame->Reserved[0])
    {
      stack_frame->Reserved[0] = 1;

      stack_frame->AddrPC.Mode = AddrModeFlat;
      stack_frame->AddrPC.Offset = context_record->Eip;
      stack_frame->AddrStack.Mode = AddrModeFlat;
      stack_frame->AddrStack.Offset = context_record->Esp;
      stack_frame->AddrFrame.Mode = AddrModeFlat;
      stack_frame->AddrFrame.Offset = context_record->Ebp;

      stack_frame->AddrReturn.Mode = AddrModeFlat;
      if (!ReadProcessMemory(process,
                             (LPCVOID) (stack_frame->AddrFrame.Offset + sizeof(DWORD)),
                             &stack_frame->AddrReturn.Offset, sizeof(DWORD), NULL))
        return FALSE;
    }
  else
    {
      stack_frame->AddrPC.Offset = stack_frame->AddrReturn.Offset;

      if (!ReadProcessMemory(process, (LPCVOID) stack_frame->AddrFrame.Offset,
                            &stack_frame->AddrFrame.Offset, sizeof(DWORD), NULL))
        return FALSE;

      if (!ReadProcessMemory(process, (LPCVOID) (stack_frame->AddrFrame.Offset + sizeof(DWORD)),
                             &stack_frame->AddrReturn.Offset, sizeof(DWORD), NULL))
        return FALSE;
    }

  ReadProcessMemory(process, (LPCVOID) (stack_frame->AddrFrame.Offset + 2*sizeof(DWORD)),
                    stack_frame->Params, sizeof(stack_frame->Params), NULL);

  return TRUE;
}

static void
unwind_stack(FILE *log, HANDLE process, PCONTEXT context)
{
  STACKFRAME          sf;

  fprintf(log, "Stack trace:\n\n");

  ZeroMemory(&sf,  sizeof(STACKFRAME));
  sf.AddrPC.Offset    = context->Eip;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrStack.Offset = context->Esp;
  sf.AddrStack.Mode   = AddrModeFlat;
  sf.AddrFrame.Offset = context->Ebp;
  sf.AddrFrame.Mode   = AddrModeFlat;

  fprintf(log, "PC        Frame     Ret\n");

  while (TRUE)
    {
      if (!stack_walk(process, &sf, context))
        break;

      if (sf.AddrFrame.Offset == 0)
        break;

      fprintf(log, "%08X  %08X  %08X\n",
              (int) sf.AddrPC.Offset,
              (int) sf.AddrFrame.Offset,
              (int) sf.AddrReturn.Offset);
    }
}

static void
print_module_info(FILE *log, HMODULE mod)
{
  TCHAR buffer[MAX_PATH];
  HANDLE file;
  SYSTEMTIME file_time;
  FILETIME write_time;

  GetModuleFileName(mod, buffer, MAX_PATH);

  file = CreateFile(buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
  if (file != INVALID_HANDLE_VALUE)
    {
      if (GetFileTime(file, NULL, NULL, &write_time))
        {
          FileTimeToSystemTime(&write_time, &file_time);
        }
      CloseHandle(file);
    }

  fprintf(log, " %-20s handle: %p date: %d-%.2d-%.2d %.2d:%.2d:%.2d\n",
          buffer,
          mod,
          file_time.wYear,
          file_time.wMonth,
          file_time.wDay,
          file_time.wHour,
          file_time.wMinute,
          file_time.wSecond
          );
}

void
print_module_list(FILE *log)
{
  HMODULE lib;
  BOOL (WINAPI *EnumProcessModules)(HANDLE, HMODULE*, DWORD, LPDWORD);

  EnumProcessModules = NULL;
  lib = LoadLibrary("psapi.dll");
  if (lib != NULL)
    {
      EnumProcessModules = (BOOL (WINAPI *)(HANDLE, HMODULE*, DWORD, LPDWORD)) GetProcAddress(lib, "EnumProcessModules");
    }

  if (EnumProcessModules != NULL)
    {
      HMODULE modules[100];
      DWORD needed;
      BOOL res;
      int count, i;

      HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
      if (proc != NULL)
        {
          res = EnumProcessModules(proc, modules, sizeof(modules), &needed);
          CloseHandle(proc);
          if (res)
            {
              count = min(needed / sizeof(HMODULE), 100);

              for (i = 0; i != count; i++)
                {
                  print_module_info(log, modules[i]);
                }
              return;
            }
        }
    }

  print_module_info(log, NULL);
}

static void
dump_registers(FILE *log, PCONTEXT context)
{
  fprintf(log, "Registers:\n\n");

  if (context->ContextFlags & CONTEXT_INTEGER)
    {
      fprintf(log, "eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
              context->Eax, context->Ebx, context->Ecx, context->Edx,
              context->Esi, context->Edi);
    }

  if (context->ContextFlags & CONTEXT_CONTROL)
    {
      fprintf(log, "eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx %s %s %s %s %s %s %s %s %s %s\n",
              context->Eip, context->Esp, context->Ebp,
              (context->EFlags >> 12) & 3,  //  IOPL level value
              context->EFlags & 0x00100000 ? "vip" : "   ", //  VIP (virtual interrupt pending)
              context->EFlags & 0x00080000 ? "vif" : "   ", //  VIF (virtual interrupt flag)
              context->EFlags & 0x00000800 ? "ov" : "nv", //  VIF (virtual interrupt flag)
              context->EFlags & 0x00000400 ? "dn" : "up", //  OF (overflow flag)
              context->EFlags & 0x00000200 ? "ei" : "di", //  IF (interrupt enable flag)
              context->EFlags & 0x00000080 ? "ng" : "pl", //  SF (sign flag)
              context->EFlags & 0x00000040 ? "zr" : "nz", //  ZF (zero flag)
              context->EFlags & 0x00000010 ? "ac" : "na", //  AF (aux carry flag)
              context->EFlags & 0x00000004 ? "po" : "pe", //  PF (parity flag)
              context->EFlags & 0x00000001 ? "cy" : "nc"  //  CF (carry flag)
              );
    }

  if (context->ContextFlags & CONTEXT_SEGMENTS)
    {
      fprintf(log, "cs=%04lx  ss=%04lx  ds=%04lx  es=%04lx  fs=%04lx  gs=%04lx",
              context->SegCs, context->SegSs, context->SegDs, context->SegEs,
              context->SegFs, context->SegGs);

      if(context->ContextFlags & CONTEXT_CONTROL)
        {
          fprintf(log, "             efl=%08lx", context->EFlags);
        }
    }
  else
    {
      if (context->ContextFlags & CONTEXT_CONTROL)
        {
          fprintf(log, "                                                                       efl=%08lx",
                  context->EFlags);
        }
    }

  fprintf(log, "\n\n");
}

static void
save_key(FILE *log, HKEY key, char *name)
{
  DWORD i;
  char keyname[512];
  int keyname_len = strlen(keyname);

  fprintf(log, "key = %s\n", name);

  for (i = 0; ; i++)
    {
      char val[256];
      DWORD val_size = sizeof(val);
      BYTE data[0x4000];
      DWORD data_size = sizeof(data);
      DWORD type;

      LONG rc = RegEnumValue(key, i, val, &val_size, 0, &type, data, &data_size);

      if (rc != ERROR_SUCCESS)
        break;

      if (val_size)
        fprintf(log, "  value = %s\n", val);

      if (strcmp("password", val) == 0)
        {
          fprintf(log, "  string data = <hidden>\n");
        }
      else if (type == REG_SZ)
        {
          fprintf(log, "  string data = %s\n", data);
        }
      else if (type == REG_DWORD && data_size==4)
        {
          fprintf(log, "  dword data = %08lx\n", (long)data);
        }
      else
        {
          fprintf(log, "  hex data = [unsupported]\n");
        }
    }

  fprintf(log, "\n");

  strcpy(keyname, name);
  strcat(keyname, "\\");
  keyname_len = strlen(keyname);

  for (i = 0; ; i++)
    {
      HKEY subkey;
      LONG rc = RegEnumKey(key, i, keyname + keyname_len,
                           sizeof(keyname) - keyname_len);

      if (rc != ERROR_SUCCESS)
        break;

      rc = RegOpenKey(key, keyname + keyname_len, &subkey);
      if (rc == ERROR_SUCCESS)
        {
          dump_registry(log, subkey, keyname);
          RegCloseKey(subkey);
        }
    }
}


static void
dump_registry(FILE *log, HKEY key, char *name)
{
  (void) key;

  HKEY handle;
  LONG rc = RegOpenKeyEx(HKEY_CURRENT_USER, name, 0, KEY_ALL_ACCESS, &handle);

  if (rc == ERROR_SUCCESS)
    {
      save_key(log, handle, name);
      RegCloseKey(handle);
    }
}
