#include <windows.h>
#include "zapper.h"

typedef DWORD (__stdcall *QUERYFULLPROCESSIMAGENAME)(HANDLE, DWORD, LPTSTR, PDWORD);
typedef DWORD (__stdcall *GETMODULEFILENAMEEX)(HANDLE, HMODULE, LPTSTR, DWORD);

static QUERYFULLPROCESSIMAGENAME pfnQueryFullProcessImageName = NULL;
static GETMODULEFILENAMEEX pfnGetModuleFileNameEx = NULL;

static int success = FALSE;

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
      if (ptr != NULL && strcmp(ptr, "Workrave.exe"))
        {
          PostMessage(hwnd, WM_ENDSESSION, 1, 0);

          success = TRUE;
          return FALSE;
        }
    }

  return TRUE;
}

int ZapWorkrave()
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

  return success;
}

