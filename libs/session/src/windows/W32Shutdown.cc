// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "W32Shutdown.hh"

#include "W32LockScreen.hh"

#include <shlobj.h>
#include <shldisp.h>

#if defined(HAVE_HARPOON)
#  include "harpoon.h"
#endif

#if !defined(HAVE_ISHELLDISPATCH)
#  undef INTERFACE
#  define INTERFACE IShellDispatch
DECLARE_INTERFACE_(IShellDispatch, IUnknown)
{
  STDMETHOD(QueryInterface)(THIS_ REFIID, PVOID *) PURE;
  STDMETHOD_(ULONG, AddRef)(THIS) PURE;
  STDMETHOD_(ULONG, Release)(THIS) PURE;
  STDMETHOD_(ULONG, dummy1)(THIS) PURE;
  STDMETHOD_(ULONG, dummy2)(THIS) PURE;
  STDMETHOD_(ULONG, dummy3)(THIS) PURE;
  STDMETHOD_(ULONG, dummy4)(THIS) PURE;
  STDMETHOD_(ULONG, dummy5)(THIS) PURE;
  STDMETHOD_(ULONG, dummy6)(THIS) PURE;
  STDMETHOD_(ULONG, dummy7)(THIS) PURE;
  STDMETHOD_(ULONG, dummy8)(THIS) PURE;
  STDMETHOD_(ULONG, dummy9)(THIS) PURE;
  STDMETHOD_(ULONG, dummya)(THIS) PURE;
  STDMETHOD_(ULONG, dummyb)(THIS) PURE;
  STDMETHOD_(ULONG, dummyc)(THIS) PURE;
  STDMETHOD_(ULONG, dummyd)(THIS) PURE;
  STDMETHOD_(ULONG, dummye)(THIS) PURE;
  STDMETHOD_(ULONG, dummyf)(THIS) PURE;
  STDMETHOD_(ULONG, dummyg)(THIS) PURE;
  STDMETHOD_(ULONG, dummyh)(THIS) PURE;
  STDMETHOD(ShutdownWindows)(THIS) PURE;
  STDMETHOD_(ULONG, dummyi)(THIS) PURE;
  STDMETHOD_(ULONG, dummyj)(THIS) PURE;
  STDMETHOD_(ULONG, dummyk)(THIS) PURE;
  STDMETHOD_(ULONG, dummyl)(THIS) PURE;
  STDMETHOD_(ULONG, dummym)(THIS) PURE;
  STDMETHOD_(ULONG, dummyn)(THIS) PURE;
  STDMETHOD_(ULONG, dummyo)(THIS) PURE;
  STDMETHOD_(ULONG, dummyp)(THIS) PURE;
  STDMETHOD_(ULONG, dummyq)(THIS) PURE;
  END_INTERFACE
};
typedef IShellDispatch *LPSHELLDISPATCH;
#endif

// uuid(D8F015C0-C278-11CE-A49E-444553540000);
const GUID IID_IShellDispatch = {0xD8F015C0, 0xc278, 0x11ce, {0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54}};
// 13709620-C279-11CE-A49E-444553540000
const GUID CLSID_Shell = {0x13709620, 0xc279, 0x11ce, {0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54}};

W32Shutdown::W32Shutdown()
{
  shutdown_supported = shutdown_helper(false);
}

bool
W32Shutdown::shutdown_helper(bool for_real)
{
  bool ret = false;
  IShellDispatch *pShellDispatch = NULL;
  if (SUCCEEDED(::CoCreateInstance(CLSID_Shell, NULL, CLSCTX_SERVER, IID_IShellDispatch, (LPVOID *)&pShellDispatch)))
    {
      ret = true;
      if (for_real)
        {
#if defined(HAVE_HARPOON)
          harpoon_unblock_input();
#endif
          pShellDispatch->ShutdownWindows();
        }
      pShellDispatch->Release();
    }
  return ret;
}

bool
W32Shutdown::shutdown()
{
  return shutdown_helper(true);
}
