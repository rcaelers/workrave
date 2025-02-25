// ClsFact.h --- CClassFactory implementation
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

#include "ClsFact.h"
#include "Guid.h"
#include "Debug.h"

CClassFactory::CClassFactory(CLSID clsid)
{
  TRACE_ENTER("CClassFactory::CClassFactory");
  m_clsidObject = clsid;
  m_ObjRefCount = 1;
  g_DllRefCount++;
  TRACE_EXIT();
}

CClassFactory::~CClassFactory()
{
  TRACE_ENTER("CClassFactory::CClassFactory");
  g_DllRefCount--;
  TRACE_MSG(g_DllRefCount);
  TRACE_EXIT();
}

STDMETHODIMP
CClassFactory::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
  *ppReturn = NULL;

  if (IsEqualIID(riid, IID_IUnknown))
    {
      *ppReturn = this;
    }

  else if (IsEqualIID(riid, IID_IClassFactory))
    {
      *ppReturn = (IClassFactory *)this;
    }

  if (*ppReturn)
    {
      (*(LPUNKNOWN *)ppReturn)->AddRef();
      return S_OK;
    }

  return E_NOINTERFACE;
}

STDMETHODIMP_(DWORD) CClassFactory::AddRef()
{
  return ++m_ObjRefCount;
}

STDMETHODIMP_(DWORD) CClassFactory::Release()
{
  if (--m_ObjRefCount == 0)
    {
      delete this;
      return 0;
    }

  return m_ObjRefCount;
}

STDMETHODIMP
CClassFactory::CreateInstance(LPUNKNOWN pUnknown, REFIID riid, LPVOID *ppObject)
{
  HRESULT hResult = E_FAIL;
  LPVOID pTemp = NULL;

  *ppObject = NULL;

  if (pUnknown != NULL)
    return CLASS_E_NOAGGREGATION;

  // create the proper object
  if (IsEqualCLSID(m_clsidObject, CLSID_WorkraveDeskBand))
    {
#if defined(HAVE_TRACING)
      Debug::init();
#endif

      CDeskBand *pDeskBand = new CDeskBand();
      if (NULL == pDeskBand)
        return E_OUTOFMEMORY;

      pTemp = pDeskBand;
    }

  if (pTemp)
    {
      // get the QueryInterface return for our return value
      hResult = ((LPUNKNOWN)pTemp)->QueryInterface(riid, ppObject);

      // call Release to decement the ref count
      ((LPUNKNOWN)pTemp)->Release();
    }

  return hResult;
}

STDMETHODIMP
CClassFactory::LockServer(BOOL)
{
  return E_NOTIMPL;
}
