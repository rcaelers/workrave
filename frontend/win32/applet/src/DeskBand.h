// DeskBand.h --- CDeskBand definitions
//
// Copyright (C) 2004 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$

#include <windows.h>
#include <shlobj.h>

#include "Globals.h"

#ifndef DESKBAND_H
#define DESKBAND_H

#define DB_CLASS_NAME (TEXT("WorkraveApplet"))

#define DB_MIN_SIZE_X   10
#define DB_MIN_SIZE_Y   10

#define IDM_COMMAND  0


class TimerBox;

class CDeskBand : public IDeskBand, 
                  public IInputObject, 
                  public IObjectWithSite,
                  public IPersistStream,
                  public IContextMenu
{
 protected:
  DWORD m_ObjRefCount;

 public:
  CDeskBand();
  ~CDeskBand();

  //IUnknown methods
  STDMETHODIMP QueryInterface(REFIID, LPVOID*);
  STDMETHODIMP_(DWORD) AddRef();
  STDMETHODIMP_(DWORD) Release();

  //IOleWindow methods
  STDMETHOD (GetWindow) (HWND*);
  STDMETHOD (ContextSensitiveHelp) (BOOL);

  //IDockingWindow methods
  STDMETHOD (ShowDW) (BOOL fShow);
  STDMETHOD (CloseDW) (DWORD dwReserved);
  STDMETHOD (ResizeBorderDW) (LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved);

  //IDeskBand methods
  STDMETHOD (GetBandInfo) (DWORD, DWORD, DESKBANDINFO*);

  //IInputObject methods
  STDMETHOD (UIActivateIO) (BOOL, LPMSG);
  STDMETHOD (HasFocusIO) (void);
  STDMETHOD (TranslateAcceleratorIO) (LPMSG);

  //IObjectWithSite methods
  STDMETHOD (SetSite) (IUnknown*);
  STDMETHOD (GetSite) (REFIID, LPVOID*);

  //IPersistStream methods
  STDMETHOD (GetClassID) (LPCLSID);
  STDMETHOD (IsDirty) (void);
  STDMETHOD (Load) (LPSTREAM);
  STDMETHOD (Save) (LPSTREAM, BOOL);
  STDMETHOD (GetSizeMax) (ULARGE_INTEGER*);

  //IContextMenu methods
  STDMETHOD (QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
  STDMETHOD (InvokeCommand)(LPCMINVOKECOMMANDINFO);
  STDMETHOD (GetCommandString)(UINT, UINT, LPUINT, LPSTR, UINT);

 private:
  BOOL m_bFocus;
  HWND m_hwndParent;
  HWND m_hWnd;
  DWORD m_dwViewMode;
  DWORD m_dwBandID;
  IInputObjectSite *m_pSite;
  TimerBox *m_TimerBox;
  
 private:
  void FocusChange(BOOL);
  LRESULT OnKillFocus(void);
  LRESULT OnSetFocus(void);
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  LRESULT OnPaint(void);
  LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
  LRESULT OnCopyData(PCOPYDATASTRUCT data);
  LRESULT OnSize(LPARAM);
  BOOL RegisterAndCreateWindow(void);
  
};

#endif   //DESKBAND_H

