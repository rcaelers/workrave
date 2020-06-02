#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "zapper.h"

#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#pragma warning(push)
#pragma warning(disable : 4100) //unreferenced formal parameter
#endif

typedef DWORD(__stdcall *QUERYFULLPROCESSIMAGENAME)(HANDLE, DWORD, LPTSTR, PDWORD);
typedef DWORD(__stdcall *GETMODULEFILENAMEEX)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef DWORD(__stdcall *GETMODULEBASENAME)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef BOOL(__stdcall *ENUMPROCESSES)(DWORD *, DWORD, DWORD *);
typedef BOOL(__stdcall *ENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);

static QUERYFULLPROCESSIMAGENAME pfnQueryFullProcessImageName = NULL;
static GETMODULEFILENAMEEX pfnGetModuleFileNameEx = NULL;
static ENUMPROCESSES pfnEnumProcesses = NULL;
static ENUMPROCESSMODULES pfnEnumProcessModules = NULL;
static GETMODULEBASENAME pfnGetModuleBaseName = NULL;

static BOOL success = FALSE;
static BOOL simulate = FALSE;

#define MAX_CLASS_NAME (128)
#define MAX_TITLE (128)

enum Kind
{
  KIND_NONE,
  KIND_EGG,
  KIND_MENU
};

static BOOL
GetProcessName(HWND hwnd, char *buf, size_t buf_size)
{
  HANDLE process_handle = NULL;
  BOOL process_name_found = FALSE;
  DWORD processid = 0;

  if (GetWindowThreadProcessId(hwnd, &processid) && processid != 0)
    {
      process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, processid);
      if (process_handle != 0)
        {
          if (pfnQueryFullProcessImageName != NULL)
            {
              DWORD size = buf_size;
              if (pfnQueryFullProcessImageName(process_handle, 0, buf, &size))
                {
                  process_name_found = TRUE;
                }
            }

          if (!process_name_found && pfnGetModuleFileNameEx)
            {
              if (pfnGetModuleFileNameEx(process_handle, NULL, buf, buf_size))
                {
                  process_name_found = TRUE;
                }
            }
          CloseHandle(process_handle);
        }
    }
  return process_name_found;
}

static void
SendQuit(HWND hwnd, enum Kind kind)
{
  switch (kind)
    {
      case KIND_NONE:
        break;

      case KIND_EGG:
        PostMessage(hwnd, WM_ENDSESSION, 1, 0);
        break;

      case KIND_MENU:
        PostMessage(hwnd, WM_USER, 14, 0);
        break;
    }
}

BOOL CALLBACK
EnumWindowsProc(HWND hwnd, long lParam)
{
  char className[MAX_CLASS_NAME] = { 0, };
  char title[MAX_TITLE] = { 0,  };
  char processName[2 * MAX_PATH] = { 0, };
  int n = 0;
  enum Kind kind = KIND_NONE;

  n = GetClassName(hwnd, (LPSTR)className, sizeof(className) - 1);
  if (n < 0)
    {
      n = 0;
    }
  className[n] = '\0';
  // printf("className = %s\n", className);

  n = GetWindowText(hwnd, (LPSTR)title, sizeof(title) - 1);
  if (n < 0)
    {
      n = 0;
    }
  title[n] = '\0';
  // printf("title = %s\n", title);
  
  if (strcmp(className, "EggSmClientWindow") == 0)
    {
      kind = KIND_EGG;
    }
  else if ((strcmp(className, "gdkWindowToplevel") == 0) && (strcmp(title, "Workrave") == 0))
    {
      kind = KIND_MENU;
    }
  // printf("kind = %d\n", kind);

  if (kind != KIND_NONE)
    {
      BOOL ret = GetProcessName(hwnd, processName, sizeof(processName));

      if (ret)
        {
          // printf("processName = %s\n", processName);
          char *ptr = strrchr(processName, '\\');
          if (ptr != NULL && _stricmp(ptr + 1, "Workrave.exe" ) == 0)
            {
              success = TRUE;
              if (!simulate)
                {
                  SendQuit(hwnd, kind);
                }
            }
        }
    }
  return !success;
}

static void
FindOrZapWorkrave(void)
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
      EnumWindows(EnumWindowsProc, 0L);
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
FindWorkrave(void)
{
  success = FALSE;
  simulate = TRUE;

  FindOrZapWorkrave();

  return success;
}

BOOL
ZapWorkrave(void)
{
  success = FALSE;
  simulate = FALSE;

  FindOrZapWorkrave();

  return success;
}

BOOL
KillProcess(char *proces_name_to_kill)
{
  HINSTANCE psapi = psapi = LoadLibrary("psapi.dll");

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
        }
      while (ret && needed > (count * sizeof(DWORD)));

      if (ret)
        {
          count = needed / sizeof(DWORD);

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
                      pfnGetModuleBaseName(process_handle, module_handle, process_name, sizeof(process_name) / sizeof(char));
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif
