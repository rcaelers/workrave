#include <windows.h>
#include <stdio.h>

#include "zapper.h"

#pragma comment(lib, "user32.lib")

#pragma warning(push)
#pragma warning(disable:4100) //unreferenced formal parameter

typedef DWORD (__stdcall *QUERYFULLPROCESSIMAGENAME)(HANDLE, DWORD, LPTSTR, PDWORD);
typedef DWORD (__stdcall *GETMODULEFILENAMEEX)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef DWORD (__stdcall *GETMODULEBASENAME)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef BOOL  (__stdcall *ENUMPROCESSES)(DWORD *, DWORD, DWORD *);
typedef BOOL  (__stdcall *ENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);

static QUERYFULLPROCESSIMAGENAME pfnQueryFullProcessImageName = NULL;
static GETMODULEFILENAMEEX       pfnGetModuleFileNameEx = NULL;
static ENUMPROCESSES             pfnEnumProcesses = NULL;
static ENUMPROCESSMODULES        pfnEnumProcessModules = NULL;
static GETMODULEBASENAME         pfnGetModuleBaseName = NULL;

static BOOL success = FALSE;
static BOOL simulate = FALSE;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, long lParam)
{
    BOOL func_retval = TRUE;
    DWORD processid = 0;
    HANDLE process_handle = NULL;
    char *ptr = NULL;
    char buf[ 2 * MAX_PATH ] = { 0, };
    BOOL process_name_found = FALSE;
    int n = 0;

    n = GetClassName( hwnd, (LPSTR)buf, sizeof( buf ) - 1 );
    if( n <= 0 )
        goto cleanup;

    buf[ n ] = '\0';

    if( strcmp( buf, "EggSmClientWindow" ) )
        goto cleanup;

    if( !GetWindowThreadProcessId( hwnd, &processid )
        || !processid
        )
        goto cleanup;

    process_handle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processid );
    if( !process_handle )
        goto cleanup;

    if( pfnQueryFullProcessImageName )
    {
        DWORD buf_size = sizeof( buf );
        if( pfnQueryFullProcessImageName( process_handle, 0, buf, &buf_size ) )
            process_name_found = TRUE;
    }

    if( !process_name_found && pfnGetModuleFileNameEx )
    {
        if( pfnGetModuleFileNameEx( process_handle, NULL, buf, sizeof( buf ) ) )
            process_name_found = TRUE;
    }

    if( !process_name_found )
        goto cleanup;

    ptr = strrchr( buf, '\\' );
    if( !ptr
        || _stricmp( ptr + 1, "Workrave.exe" )
        )
        goto cleanup;

    if( !simulate )
        PostMessage( hwnd, WM_ENDSESSION, 1, 0 );

    success = TRUE;
    func_retval = FALSE;

cleanup:

    if( process_handle )
        CloseHandle( process_handle );

    return func_retval;
}


static void
FindOrZapWorkrave()
{
  HINSTANCE psapi = NULL;
  HINSTANCE kernel32 = LoadLibrary("kernel32.dll");

  if (kernel32 != NULL)
    {
      pfnQueryFullProcessImageName = (QUERYFULLPROCESSIMAGENAME)GetProcAddress(kernel32, "QueryFullProcessImageNameA");
    }

  if (pfnQueryFullProcessImageName == NULL)
    {
      psapi = LoadLibrary("psapi.dll");
      pfnGetModuleFileNameEx = (GETMODULEFILENAMEEX)GetProcAddress(psapi, "GetModuleFileNameExA");
    }

  if (pfnQueryFullProcessImageName != NULL || pfnGetModuleFileNameEx != NULL)
    {
      EnumWindows(EnumWindowsProc, 0);
    }

  if (kernel32 != NULL)
    {
      FreeLibrary(kernel32);
    }

  if (psapi != NULL)
    {
      FreeLibrary(psapi);
    }
}


BOOL
FindWorkrave()
{
  success = FALSE;
  simulate = TRUE;

  FindOrZapWorkrave();

  return success;
}

BOOL
ZapWorkrave()
{
  success = FALSE;
  simulate = FALSE;

  FindOrZapWorkrave();

  return success;
}

BOOL
KillProcess(char *proces_name_to_kill)
{
  HINSTANCE psapi =  psapi = LoadLibrary("psapi.dll");
  BOOL ret = FALSE;

  pfnEnumProcesses = (ENUMPROCESSES)GetProcAddress(psapi, "EnumProcesses");
  pfnGetModuleBaseName = (GETMODULEBASENAME)GetProcAddress(psapi, "GetModuleBaseNameA");
  pfnEnumProcessModules = (ENUMPROCESSMODULES)GetProcAddress(psapi, "EnumProcessModules");

  if (pfnEnumProcesses != NULL && pfnGetModuleBaseName != NULL && pfnEnumProcessModules != NULL)
    {
      DWORD i = 0;
      DWORD count = 0;
      DWORD needed = 0;
      DWORD *pids = NULL;
      BOOL ret = FALSE;

      do
        {
          count += 1024;
          pids = realloc(pids, count * sizeof(DWORD));
          ret = pfnEnumProcesses(pids, count * sizeof(DWORD), &needed);

        } while (ret && needed > (count * sizeof(DWORD)));

      if (ret)
        {
          count = needed / sizeof (DWORD);

          for (i = 0; i < count; i++)
            {
              DWORD pid = pids[i];
              HANDLE process_handle = NULL;
              char process_name[MAX_PATH] = "";

              if (pid == 0)
                {
                  continue;
                }

              process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
              if (process_handle != NULL)
                {
                  HMODULE module_handle;
                  DWORD cbNeeded;

                  if (pfnEnumProcessModules(process_handle, &module_handle, sizeof(module_handle), &cbNeeded))
                    {
                      pfnGetModuleBaseName(process_handle, module_handle, process_name, sizeof(process_name)/sizeof(char));
                    }

                  if (_stricmp(process_name, proces_name_to_kill) == 0)
                    {
                      TerminateProcess(process_handle, (UINT)-1);
                      CloseHandle(process_handle);
                      break;
                    }

                  CloseHandle(process_handle);
                }
            }
        }
      free(pids);
      pids = NULL;
    }

  if (psapi != NULL)
    {
      FreeLibrary(psapi);
    }

  return ret;
}

#pragma warning(pop)
