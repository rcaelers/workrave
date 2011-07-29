#include <windows.h>
#include <stdio.h>

#include "zapper.h"

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
  char buf[2 * MAX_PATH] = { 0 };

  int n = GetClassName(hwnd, (LPSTR) buf, sizeof(buf) - 1);
  buf[n] = '\0';

  if (strcmp(buf, "EggSmClientWindow") == 0)
    {
      DWORD processid = 0;
      GetWindowThreadProcessId(hwnd, &processid);

      HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processid);
      if (process_handle != NULL)
        {
          if (pfnQueryFullProcessImageName != NULL)
            {
              DWORD buf_size = sizeof(buf);
              pfnQueryFullProcessImageName(process_handle, 0, buf, &buf_size);
            }
          else if (pfnGetModuleFileNameEx != NULL)
            {
              pfnGetModuleFileNameEx(process_handle, NULL, buf, sizeof(buf));
            }

          CloseHandle(process_handle);
        }

      char *ptr = strrchr(buf, '\\');
      if (ptr != NULL && stricmp(ptr + 1, "Workrave.exe") == 0)
        {
          if (! simulate)
            {
              PostMessage(hwnd, WM_ENDSESSION, 1, 0);
            }

          success = TRUE;
          return FALSE;
        }
    }

  return TRUE;
}


static void
FindOrZapWorkrave()
{
  HINSTANCE kernel32 = LoadLibrary("kernel32.dll");
  if (kernel32 != NULL)
    {
      pfnQueryFullProcessImageName = (QUERYFULLPROCESSIMAGENAME)GetProcAddress(kernel32, "QueryFullProcessImageNameA");
    }

  HINSTANCE psapi = NULL;
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
      int i = 0;
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
              if (pid == 0)
                {
                  continue;
                }

              char process_name[MAX_PATH] = "";

              HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
              if (process_handle != NULL)
                {
                  HMODULE module_handle;
                  DWORD cbNeeded;

                  if (pfnEnumProcessModules(process_handle, &module_handle, sizeof(module_handle), &cbNeeded))
                    {
                      pfnGetModuleBaseName(process_handle, module_handle, process_name, sizeof(process_name)/sizeof(char));
                    }

                  if (stricmp(process_name, proces_name_to_kill) == 0)
                    {
                      TerminateProcess(process_handle, -1);
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
