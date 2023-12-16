#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <tlhelp32.h>
#include <psapi.h>
#include <stdbool.h>
#include <string.h>

#include "zapper.h"

#if defined(_MSC_VER)
#  pragma comment(lib, "user32.lib")

#  pragma warning(push)
#  pragma warning(disable : 4100) // unreferenced formal parameter
#endif

typedef DWORD(__stdcall *QUERYFULLPROCESSIMAGENAME)(HANDLE, DWORD, LPTSTR, PDWORD);
typedef DWORD(__stdcall *GETMODULEFILENAMEEX)(HANDLE, HMODULE, LPTSTR, DWORD);

static QUERYFULLPROCESSIMAGENAME pfnQueryFullProcessImageName = NULL;
static GETMODULEFILENAMEEX pfnGetModuleFileNameEx = NULL;

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

static BOOL CALLBACK
EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  char className[MAX_CLASS_NAME] = {
    0,
  };
  char title[MAX_TITLE] = {
    0,
  };
  char processName[2 * MAX_PATH] = {
    0,
  };
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
          if (ptr != NULL && _stricmp(ptr + 1, "Workrave.exe") == 0)
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

static bool
IsStringInList(const char *str, const char **list)
{
  while (*list != NULL)
    {
      if (_stricmp(str, *list) == 0)
        {
          return true;
        }
      list++;
    }
  return false;
}

static bool
MatchProcess(DWORD process_id, const char *directory, const char **allowed_executable_names)
{
  HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);

  if (process_handle != NULL)
    {
      char process_path[MAX_PATH] = {0};
      if (GetModuleFileNameEx(process_handle, NULL, process_path, MAX_PATH) > 0)
        {
          if (strstr(process_path, directory) == process_path)
            {
              char *executable_name = strrchr(process_path, '\\');
              if (executable_name != NULL)
                {
                  executable_name++;
                  if (IsStringInList(executable_name, allowed_executable_names))
                    {
                      CloseHandle(process_handle);
                      return true;
                    }
                }
            }
        }
      CloseHandle(process_handle);
    }
  return false;
}



int
TerminateProcessesByNames(const char *directory, const char **executable_names_to_kill, bool dry_run)
{
  HANDLE process_snapshot_handle = NULL;
  PROCESSENTRY32 process_entry;

  process_snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (process_snapshot_handle == INVALID_HANDLE_VALUE)
    {
      return -1;
    }

  process_entry.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(process_snapshot_handle, &process_entry))
    {
      CloseHandle(process_snapshot_handle);
      return -1;
    }

  int ret = 0;
  do
    {
      if (MatchProcess(process_entry.th32ProcessID, directory, executable_names_to_kill))
        {
          ret = 1;
          if (!dry_run)
            {
              HANDLE process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, process_entry.th32ProcessID);
              if (process_handle != NULL)
                {
                  TerminateProcess(process_handle, 1);
                  CloseHandle(process_handle);
                }
            }
        }
    }
  while (Process32Next(process_snapshot_handle, &process_entry));

  CloseHandle(process_snapshot_handle);
  return ret;
}

static const char *workrave_executables[] =
  {"Workrave.exe", "workrave.exe", "WorkraveHelper.exe", "gdbus.exe", "harpoonHelper.exe", "dbus-daemon.exe", NULL};

BOOL
AreWorkraveProcessesRunning(const char *directory)
{
  return TerminateProcessesByNames(directory, workrave_executables, true) == 1;
}

BOOL
KillWorkraveProcesses(const char *directory)
{
  return TerminateProcessesByNames(directory, workrave_executables, false) != -1;
}

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
