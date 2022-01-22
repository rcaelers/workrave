// DeskBand.h --- CDeskBand definitions
//
// Copyright (C) 2004, 2005, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#include <windows.h>
#include <shlobj.h>
#include <time.h>
#include <string>

#include "Globals.h"
#include "Applet.hh"

#ifndef DESKBAND_H
#  define DESKBAND_H

#  define DB_CLASS_NAME (TEXT("WorkraveApplet"))

#  define DB_MIN_SIZE_X 10
#  define DB_MIN_SIZE_Y 10

class TimerBox;

class CDeskBand
  : public IDeskBand2
  , public IInputObject
  , public IObjectWithSite
  , public IPersistStream
  , public IContextMenu
{
protected:
  DWORD m_ObjRefCount{1};

public:
  CDeskBand();
  virtual ~CDeskBand();

  // IUnknown methods
  STDMETHODIMP QueryInterface(REFIID, LPVOID *);
  STDMETHODIMP_(DWORD) AddRef();
  STDMETHODIMP_(DWORD) Release();

  // IOleWindow methods
  STDMETHOD(GetWindow)(HWND *);
  STDMETHOD(ContextSensitiveHelp)(BOOL);

  // IDockingWindow methods
  STDMETHOD(ShowDW)(BOOL fShow);
  STDMETHOD(CloseDW)(DWORD dwReserved);
  STDMETHOD(ResizeBorderDW)(LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved);

  // IDeskBand methods
  STDMETHOD(GetBandInfo)(DWORD, DWORD, DESKBANDINFO *);

  // IDeskBand2 methods
  STDMETHOD(CanRenderComposited)(BOOL *);
  STDMETHOD(GetCompositionState)(BOOL *);
  STDMETHOD(SetCompositionState)(BOOL);

  // IInputObject methods
  STDMETHOD(UIActivateIO)(BOOL, LPMSG);
  STDMETHOD(HasFocusIO)();
  STDMETHOD(TranslateAcceleratorIO)(LPMSG);

  // IObjectWithSite methods
  STDMETHOD(SetSite)(IUnknown *);
  STDMETHOD(GetSite)(REFIID, LPVOID *);

  // IPersistStream methods
  STDMETHOD(GetClassID)(LPCLSID);
  STDMETHOD(IsDirty)();
  STDMETHOD(Load)(LPSTREAM);
  STDMETHOD(Save)(LPSTREAM, BOOL);
  STDMETHOD(GetSizeMax)(ULARGE_INTEGER *);

  // IContextMenu methods
  STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
  STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
  STDMETHOD(GetCommandString)(UINT_PTR, UINT, LPUINT, LPSTR, UINT);

  HWND get_command_window() const;

private:
  BOOL m_bFocus{FALSE};
  HWND m_hwndParent{nullptr};
  HWND m_hWnd{nullptr};
  DWORD m_dwViewMode{0};
  DWORD m_dwBandID{0};
  IInputObjectSite *m_pSite{nullptr};
  TimerBox *m_TimerBox{nullptr};
  time_t m_LastCopyData{0};
  AppletMenuData m_AppletMenu{};
  BOOL m_HasAppletMenu{FALSE};
  BOOL m_CompositionEnabled{FALSE};
  int m_preferredWidth{DB_MIN_SIZE_X};
  int m_preferredHeight{DB_MIN_SIZE_Y};
  int m_minimumWidth{DB_MIN_SIZE_X};
  int m_minimumHeight{DB_MIN_SIZE_Y};
  int current_dpi{0};

private:
  void FocusChange(BOOL);
  LRESULT OnKillFocus();
  LRESULT OnSetFocus();
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
  LRESULT OnCopyData(PCOPYDATASTRUCT data);
  LRESULT OnSize(LPARAM);
  LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
  LRESULT OnWindowPosChanging(WPARAM wParam, LPARAM lParam);
  LRESULT OnDPIChanged();
  BOOL RegisterAndCreateWindow();
  void UpdateDeskband();
  std::wstring ConvertAnsiToWide(const std::string &str);
};

inline HWND
CDeskBand::get_command_window() const
{
  return (HWND)LongToHandle(m_AppletMenu.command_window);
}

#endif // DESKBAND_H
