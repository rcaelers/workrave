#include <windows.h>
#include "zapper.h"

typedef DWORD (__stdcall *PFNQUERYFULLPROCESSIMAGENAME)(HANDLE, DWORD, LPTSTR, PDWORD);
typedef DWORD (__stdcall *PFNGETMODULEFILENAMEEX)(HANDLE, HMODULE, LPTSTR, DWORD);

static PFNQUERYFULLPROCESSIMAGENAME pfnQueryFullProcessImageName = NULL;
static PFNGETMODULEFILENAMEEX pfnGetModuleFileNameEx = NULL;

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
              DWORD buf_size = sizeof(buf) - 1;
              pfnQueryFullProcessImageName(process_handle, 0, buf, &buf_size);
            }
          else if (pfnGetModuleFileNameEx != NULL)
            {
              pfnGetModuleFileNameEx(process_handle, NULL, buf, sizeof(buf) - 1);
            }

          CloseHandle(process_handle);
        }

      char *ptr = strrchr(buf, '\\');
      if (ptr != NULL && strcmp(ptr, "Workrave.exe"))
        {
          PostMessage(hwnd,  WM_ENDSESSION, 1, 0);

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
      pfnQueryFullProcessImageName = (PFNQUERYFULLPROCESSIMAGENAME)GetProcAddress(kernel32, "QueryFullProcessImageNameA");
      FreeLibrary(kernel32);
    }

  if (pfnQueryFullProcessImageName == NULL )
    {
      HINSTANCE psapi = LoadLibrary("psapi.dll");
      pfnGetModuleFileNameEx = (PFNGETMODULEFILENAMEEX)GetProcAddress(psapi, "GetModuleFileNameExA");
      FreeLibrary(psapi);
    }

  if (pfnQueryFullProcessImageName != NULL || pfnGetModuleFileNameEx != NULL)
    {
      EnumWindows(EnumWindowsProc, 0);
    }

  return success;
}

