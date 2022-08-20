// Applet.cpp --- COM stuff
//
// Copyright (C) 2004, 2007 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $Id$

#include <ole2.h>
#include <comcat.h>
#include <olectl.h>
#include "ClsFact.h"
#include <shlwapi.h>

#if defined(_MSC_VER)
#  pragma data_seg(".text")
#endif

#define INITGUID
#include <initguid.h>
#include "Guid.h"

#include "Debug.h"

#if defined(_MSC_VER)
#  pragma data_seg()
#endif

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);
BOOL RegisterServer(CLSID, LPCTSTR, BOOL reg);
BOOL RegisterComCat(CLSID, CATID, BOOL reg);

HINSTANCE g_hInst;
UINT g_DllRefCount;

extern "C" BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
      g_hInst = hInstance;
      break;

    case DLL_PROCESS_DETACH:
      break;
    }

  return TRUE;
}

STDAPI
DllCanUnloadNow()
{
  TRACE_ENTER("DllCanUnloadNow");
  TRACE_RETURN(g_DllRefCount);
  return (g_DllRefCount ? S_FALSE : S_OK);
}

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppReturn)
{
  *ppReturn = NULL;

  // if we don't support this classid, return the proper error code
  if (!IsEqualCLSID(rclsid, CLSID_WorkraveDeskBand))
    return CLASS_E_CLASSNOTAVAILABLE;

  // create a CClassFactory object and check it for validity
  CClassFactory *pClassFactory = new CClassFactory(rclsid);
  if (NULL == pClassFactory)
    return E_OUTOFMEMORY;

  // get the QueryInterface return for our return value
  HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);

  // call Release to decement the ref count - creating the object set it to one
  // and QueryInterface incremented it - since its being used externally (not by
  // us), we only want the ref count to be 1
  pClassFactory->Release();

  // return the result from QueryInterface
  return hResult;
}

static void
ClearDeskBandCache()
{
  /*
  Remove the cache of the deskbands on Windows 2000. This will cause the new
  deskband to be displayed in the toolbar menu the next time the user brings it
  up. See KB article Q214842 for more information on this.
  */
  TCHAR szSubKey[MAX_PATH];
  TCHAR szCATID[MAX_PATH];
  LPWSTR pwszCATID;

  StringFromCLSID(CATID_DeskBand, &pwszCATID);
  if (pwszCATID)
    {
#if defined(UNICODE)
      lstrcpy(szCATID, pwszCATID);
#else
      WideCharToMultiByte(CP_ACP, 0, pwszCATID, -1, szCATID, ARRAYSIZE(szCATID), NULL, NULL);
#endif

      // free the string
      CoTaskMemFree(pwszCATID);

      wsprintf(szSubKey, TEXT("Component Categories\\%s\\Enum"), szCATID);

      RegDeleteKey(HKEY_CLASSES_ROOT, szSubKey);
    }
}

STDAPI
DllUnregisterServer()
{
  RegisterComCat(CLSID_WorkraveDeskBand, CATID_DeskBand, FALSE);
  RegisterServer(CLSID_WorkraveDeskBand, TEXT("Workrave"), FALSE);
  ClearDeskBandCache();
  return S_OK;
}

STDAPI
DllRegisterServer()
{
  // Register the desk band object.
  if (!RegisterServer(CLSID_WorkraveDeskBand, TEXT("Workrave"), TRUE))
    return SELFREG_E_CLASS;

  // Register the component categories for the desk band object.
  if (!RegisterComCat(CLSID_WorkraveDeskBand, CATID_DeskBand, TRUE))
    return SELFREG_E_CLASS;

  ClearDeskBandCache();

  return S_OK;
}

typedef struct
{
  HKEY hRootKey;
  LPCTSTR szSubKey; // TCHAR szSubKey[MAX_PATH];
  LPCTSTR lpszValueName;
  LPCTSTR szData; // TCHAR szData[MAX_PATH];
} DOREGSTRUCT, *LPDOREGSTRUCT;

BOOL
RegisterServer(CLSID clsid, LPCTSTR lpszTitle, BOOL reg)
{
  HKEY hKey;
  LRESULT lResult;
  DWORD dwDisp;
  TCHAR szSubKey[MAX_PATH];
  TCHAR szCLSID[MAX_PATH];
  TCHAR szModule[MAX_PATH];
  LPWSTR pwsz;

  // get the CLSID in string form
  StringFromIID(clsid, &pwsz);

  if (pwsz)
    {
#if defined(UNICODE)
      lstrcpy(szCLSID, pwsz);
#else
      WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szCLSID, ARRAYSIZE(szCLSID), NULL, NULL);
#endif

      // free the string
      CoTaskMemFree(pwsz);
    }

  // get this app's path and file name
  GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

  DOREGSTRUCT ClsidEntries[] = {
    {HKEY_CLASSES_ROOT, TEXT("CLSID\\%s"), NULL, lpszTitle},
    {HKEY_CLASSES_ROOT, TEXT("CLSID\\%s\\InprocServer32"), NULL, szModule},
    {HKEY_CLASSES_ROOT, TEXT("CLSID\\%s\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Apartment")}};

  if (reg)
    {
      // register the CLSID entries
      for (size_t i = 0; i < sizeof(ClsidEntries) / sizeof(ClsidEntries[0]); i++)
        {
          // create the sub key string - for this case, insert the file extension
          wsprintf(szSubKey, ClsidEntries[i].szSubKey, szCLSID);

          lResult = RegCreateKeyEx(ClsidEntries[i].hRootKey,
                                   szSubKey,
                                   0,
                                   NULL,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_WRITE,
                                   NULL,
                                   &hKey,
                                   &dwDisp);

          if (NOERROR == lResult)
            {
              TCHAR szData[MAX_PATH];

              // if necessary, create the value string
              wsprintf(szData, ClsidEntries[i].szData, szModule);

              lResult = RegSetValueEx(hKey, ClsidEntries[i].lpszValueName, 0, REG_SZ, (const BYTE *)szData, lstrlen(szData) + 1);

              RegCloseKey(hKey);
            }
          else
            return FALSE;
        }
    }
  else
    {
      // create the sub key string - for this case, insert the file extension
      wsprintf(szSubKey, ClsidEntries[0].szSubKey, szCLSID);

      SHDeleteKey(ClsidEntries[0].hRootKey, szSubKey);
    }

  // If running on NT, register the extension as approved.
  OSVERSIONINFO osvi;

  osvi.dwOSVersionInfoSize = sizeof(osvi);
  GetVersionEx(&osvi);

  if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
    {
      lstrcpy(szSubKey, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));

      lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);

      if (NOERROR == lResult)
        {
          TCHAR szData[MAX_PATH];

          // Create the value string.
          lstrcpy(szData, lpszTitle);

          if (reg)
            {
              lResult = RegSetValueEx(hKey, szCLSID, 0, REG_SZ, (LPBYTE)szData, (lstrlen(szData) + 1) * sizeof(TCHAR));
            }
          else
            {
              lResult = RegDeleteValue(hKey, szCLSID);
            }

          RegCloseKey(hKey);
        }
      else
        return FALSE;
    }

  return TRUE;
}

BOOL
RegisterComCat(CLSID clsid, CATID CatID, BOOL reg)
{
  ICatRegister *pcr;
  HRESULT hr = S_OK;

  CoInitialize(NULL);

  hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (LPVOID *)&pcr);

  if (SUCCEEDED(hr))
    {
      if (reg)
        {
          hr = pcr->RegisterClassImplCategories(clsid, 1, &CatID);
        }
      else
        {
          hr = pcr->UnRegisterClassImplCategories(clsid, 1, &CatID);
        }

      pcr->Release();
    }

  CoUninitialize();

  return SUCCEEDED(hr);
}
