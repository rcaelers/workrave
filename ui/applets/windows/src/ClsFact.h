// ClsFact.h --- CClassFactory definitions.
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

#ifndef CLASSFACTORY_H
#define CLASSFACTORY_H

#include <windows.h>
#include "Globals.h"
#include "DeskBand.h"

/**************************************************************************

   CClassFactory class definition

**************************************************************************/

class CClassFactory : public IClassFactory
{
protected:
  DWORD m_ObjRefCount;

public:
  CClassFactory(CLSID);
  virtual ~CClassFactory();

  // IUnknown methods
  STDMETHODIMP QueryInterface(REFIID, LPVOID *);
  STDMETHODIMP_(DWORD) AddRef();
  STDMETHODIMP_(DWORD) Release();

  // IClassFactory methods
  STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
  STDMETHODIMP LockServer(BOOL);

private:
  CLSID m_clsidObject;
};

#endif // CLASSFACTORY_H
