// DeskBand.cpp --- CDeskBand implementation
//
// Copyright (C) 2004, 2005, 2006, 2007, 2010 Raymond Penners <raymond@dotsphinx.com>
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

#include <windowsx.h>

#include "DeskBand.h"
#include "TimeBar.h"
#include "TimerBox.h"
#include "Guid.h"
#include "Applet.hh"
#include "Debug.h"
#include "PaintHelper.h"

CDeskBand::CDeskBand()
{
  m_pSite = NULL;
  m_hWnd = NULL;
  m_hwndParent = NULL;
  m_bFocus = FALSE;
  m_dwViewMode = 0;
  m_dwBandID = 0;
  m_TimerBox = NULL;
  m_HasAppletMenu = FALSE;
  m_LastCopyData = 0;
  m_ObjRefCount = 1;

  g_DllRefCount++;
}


CDeskBand::~CDeskBand()
{
  TRACE_ENTER("CDeskBand::~CDeskBand");
  //this should have been freed in a call to SetSite(NULL), but just to be safe
  if (m_pSite)
    {
      m_pSite->Release();
      m_pSite = NULL;
    }

  g_DllRefCount--;
  TRACE_MSG(g_DllRefCount);
  TRACE_EXIT();
}

STDMETHODIMP
CDeskBand::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
  *ppReturn = NULL;

  //IUnknown
  if (IsEqualIID(riid, IID_IUnknown))
    {
      *ppReturn = this;
    }

  //IOleWindow
  else if (IsEqualIID(riid, IID_IOleWindow))
    {
      *ppReturn = (IOleWindow*)this;
    }

  //IDockingWindow
  else if (IsEqualIID(riid, IID_IDockingWindow))
    {
      *ppReturn = (IDockingWindow*)this;
    }

  //IInputObject
  else if (IsEqualIID(riid, IID_IInputObject))
    {
      *ppReturn = (IInputObject*)this;
    }

  //IObjectWithSite
  else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
      *ppReturn = (IObjectWithSite*)this;
    }

  //IDeskBand
  else if (IsEqualIID(riid, IID_IDeskBand))
    {
      *ppReturn = (IDeskBand*)this;
    }

  //IDeskBand2
  else if (IsEqualIID(riid, IID_IDeskBand2))
    {
      *ppReturn = (IDeskBand2*)this;
    }

  //IPersist
  else if (IsEqualIID(riid, IID_IPersist))
    {
      *ppReturn = (IPersist*)this;
    }

  //IPersistStream
  else if (IsEqualIID(riid, IID_IPersistStream))
    {
      *ppReturn = (IPersistStream*)this;
    }

  //IContextMenu
  else if (IsEqualIID(riid, IID_IContextMenu))
    {
      *ppReturn = (IContextMenu*)this;
    }

  if (*ppReturn)
    {
      (*(LPUNKNOWN*)ppReturn)->AddRef();
      return S_OK;
    }

  return E_NOINTERFACE;
}

STDMETHODIMP_(DWORD)
CDeskBand::AddRef()
{
  return ++m_ObjRefCount;
}


STDMETHODIMP_(DWORD)
CDeskBand::Release()
{
  if (--m_ObjRefCount == 0)
    {
      delete this;
      return 0;
    }

  return m_ObjRefCount;
}

STDMETHODIMP
CDeskBand::GetWindow(HWND *phWnd)
{
  TRACE_ENTER("CDeskBand::GetWindow");
  *phWnd = m_hWnd;

  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::ContextSensitiveHelp(BOOL fEnterMode)
{
  TRACE_ENTER_MSG("CDeskBand::ContextSensitiveHelp",fEnterMode);
  TRACE_EXIT();
  return E_NOTIMPL;
}

STDMETHODIMP
CDeskBand::ShowDW(BOOL fShow)
{
  TRACE_ENTER_MSG("CDeskBand::ShowDW", fShow);
  if (m_hWnd)
    {
      if (fShow)
        {
          //show our window
          ShowWindow(m_hWnd, SW_SHOW);
        }
      else
        {
          //hide our window
          ShowWindow(m_hWnd, SW_HIDE);
        }
    }

  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::CloseDW(DWORD dwReserved)
{
  TRACE_ENTER("CDeskBand::CloseDW");
  ShowDW(FALSE);

  delete m_TimerBox;
  m_TimerBox = NULL;

  if (IsWindow(m_hWnd))
    DestroyWindow(m_hWnd);

  m_hWnd = NULL;

  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::ResizeBorderDW(LPCRECT prcBorder,
                          IUnknown* punkSite,
                          BOOL fReserved)
{
  TRACE_ENTER("CDeskBand::ResizeBorderDW");
  TRACE_EXIT();
  return E_NOTIMPL;
}

STDMETHODIMP
CDeskBand::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
  TRACE_ENTER_MSG("CDeskBand::UIActivateIO", fActivate);

  if (fActivate)
    SetFocus(m_hWnd);

  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::HasFocusIO()
{
  TRACE_ENTER("CDeskBand::HasFocusIO");

  if (m_bFocus)
  {
    TRACE_RETURN("OK");
    return S_OK;
  }

  TRACE_RETURN("FALSE");
  return S_FALSE;
}

STDMETHODIMP
CDeskBand::TranslateAcceleratorIO(LPMSG pMsg)
{
  TRACE_ENTER("CDeskBand::TranslateAcceleratorIO");
  TRACE_EXIT();
  return S_FALSE;
}

STDMETHODIMP
CDeskBand::SetSite(IUnknown* punkSite)
{
  TRACE_ENTER("CDeskBand::SetSite");
  //If a site is being held, release it.
  if (m_pSite)
    {
      m_pSite->Release();
      m_pSite = NULL;
    }

  //If punkSite is not NULL, a new site is being set.
  if (punkSite)
    {
      //Get the parent window.
      IOleWindow  *pOleWindow;

      m_hwndParent = NULL;

      if (SUCCEEDED(punkSite->QueryInterface(IID_IOleWindow, (LPVOID*)&pOleWindow)))
        {
          pOleWindow->GetWindow(&m_hwndParent);
          pOleWindow->Release();
        }

      if (!m_hwndParent)
        return E_FAIL;

      if (!RegisterAndCreateWindow())
        return E_FAIL;

      //Get and keep the IInputObjectSite pointer.
      if (SUCCEEDED(punkSite->QueryInterface(IID_IInputObjectSite, (LPVOID*)&m_pSite)))
        {
          return S_OK;
        }

      return E_FAIL;
    }

  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::GetSite(REFIID riid, LPVOID *ppvReturn)
{
  TRACE_ENTER("CDeskBand::GetSite");
  *ppvReturn = NULL;

  if (m_pSite)
    return m_pSite->QueryInterface(riid, ppvReturn);

  TRACE_EXIT();
  return E_FAIL;
}

STDMETHODIMP
CDeskBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
  TRACE_ENTER_MSG("CDeskBand::GetBandInfo", dwBandID << " " << dwViewMode);
  if (pdbi)
    {
      m_dwBandID = dwBandID;
      m_dwViewMode = dwViewMode;

      if (pdbi->dwMask & DBIM_MINSIZE)
        {
          if (DBIF_VIEWMODE_FLOATING & dwViewMode)
            {
              pdbi->ptMinSize.x = 200;
              pdbi->ptMinSize.y = 400;
            }
          else
            {
              pdbi->ptMinSize.x = DB_MIN_SIZE_X;
              pdbi->ptMinSize.y = DB_MIN_SIZE_Y;
            }
        }

      if (pdbi->dwMask & DBIM_MAXSIZE)
        {
          pdbi->ptMaxSize.x = -1;
          pdbi->ptMaxSize.y = -1;
        }

      if (pdbi->dwMask & DBIM_INTEGRAL)
        {
          pdbi->ptIntegral.x = 1;
          pdbi->ptIntegral.y = 1;
        }

      if (pdbi->dwMask & DBIM_ACTUAL)
        {
          pdbi->ptActual.x = 100;
          pdbi->ptActual.y = 100;
        }

      if (pdbi->dwMask & DBIM_TITLE)
        {
          if (dwViewMode & DBIF_VIEWMODE_FLOATING)
            {
              lstrcpyW(pdbi->wszTitle, L"Workrave");
            }
          else
            {
              pdbi->dwMask &= ~DBIM_TITLE;
            }
        }

      if (pdbi->dwMask & DBIM_MODEFLAGS)
        {
          pdbi->dwModeFlags = DBIMF_NORMAL;
          pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
        }

      if (pdbi->dwMask & DBIM_BKCOLOR)
        {
          //Use the default background color by removing this flag.
          pdbi->dwMask &= ~DBIM_BKCOLOR;
        }

      return S_OK;
    }

  TRACE_EXIT();
  return E_INVALIDARG;
}

STDMETHODIMP
CDeskBand::CanRenderComposited(BOOL *pfCanRenderComposited)
{
  TRACE_ENTER("CDeskBand::CanRenderComposited");

  if (!pfCanRenderComposited)
    return E_INVALIDARG;
  
  *pfCanRenderComposited = TRUE;
  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::GetCompositionState( BOOL *pfCompositionEnabled )
{
  TRACE_ENTER("CDeskBand::GetCompositionState");
  if (!pfCompositionEnabled)
    return E_INVALIDARG;
  
  *pfCompositionEnabled = m_CompositionEnabled;
  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::SetCompositionState( BOOL fCompositionEnabled )
{
  TRACE_ENTER_MSG("CDeskBand::SetCompositionState", fCompositionEnabled);
  m_CompositionEnabled = fCompositionEnabled;

  PaintHelper::SetCompositionEnabled(fCompositionEnabled != FALSE);
  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::GetClassID(LPCLSID pClassID)
{
  *pClassID = CLSID_WorkraveDeskBand;

  return S_OK;
}

STDMETHODIMP
CDeskBand::IsDirty()
{
  TRACE_ENTER("CDeskBand::IsDirty");
  TRACE_EXIT();
  return S_FALSE;
}

STDMETHODIMP
CDeskBand::Load(LPSTREAM pStream)
{
  TRACE_ENTER("CDeskBand::Load");
  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::Save(LPSTREAM pStream, BOOL fClearDirty)
{
  TRACE_ENTER("CDeskBand::Save");
  TRACE_EXIT();
  return S_OK;
}

STDMETHODIMP
CDeskBand::GetSizeMax(ULARGE_INTEGER *pul)
{
  TRACE_ENTER("CDeskBand::GetSizeMax");
  TRACE_EXIT();
  return E_NOTIMPL;
}

STDMETHODIMP
CDeskBand::QueryContextMenu(HMENU hMenu,
                            UINT indexMenu,
                            UINT idCmdFirst,
                            UINT idCmdLast,
                            UINT uFlags)
{
  TRACE_ENTER("CDeskBand::QueryContextMenu");
  if ((!m_HasAppletMenu) || (CMF_DEFAULTONLY & uFlags) || !IsWindow( get_command_window() ))
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));

  int m = 0;
  HMENU popup = NULL;
  while (m < m_AppletMenu.num_items)
    {
      AppletMenuItemData *d = &m_AppletMenu.items[m];
      wchar_t textw[APPLET_MENU_TEXT_MAX_LENGTH*2];
      MultiByteToWideChar(CP_UTF8, 0, d->text, -1,
                          textw, sizeof(textw)/sizeof(textw[0]));
      wchar_t *abbrev = wcschr(textw, '_');
      UINT flags = MF_STRING | MF_BYPOSITION;
      if (d->flags & APPLET_MENU_FLAG_SELECTED)
        {
          flags |= MF_CHECKED;
        }
      if (abbrev != NULL)
        {
          *abbrev = '&';
        }
      if (d->flags & APPLET_MENU_FLAG_POPUP)
        {
          if (popup == NULL)
            {
              popup = CreatePopupMenu();
            }
          AppendMenuW(popup,
                      flags,
                      idCmdFirst + m,
                      textw);
        }
      else
        {
          if (popup != NULL)
            {
              InsertMenuW(hMenu,
                          indexMenu++,
                          MF_POPUP | flags,
                          (UINT_PTR) popup,
                          textw);
              popup = NULL;
            }
          else
            {
              InsertMenuW(hMenu,
                          indexMenu++,
                          flags,
                          idCmdFirst + m,
                          textw);
            }
        }
      m++;
    }

  TRACE_EXIT();
  return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(m_AppletMenu.num_items + 1));
}

STDMETHODIMP
CDeskBand::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
  TRACE_ENTER("CDeskBand::InvokeCommand");
  int cmd = LOWORD(lpcmi->lpVerb);
  HRESULT ret;

  if (m_HasAppletMenu && cmd >= 0 && cmd < m_AppletMenu.num_items && IsWindow( get_command_window() ))
    {
      SendMessage( get_command_window(), WM_USER, m_AppletMenu.items[cmd].command, NULL );
      ret = NOERROR;
    }
  else
    {
      ret = E_INVALIDARG;
    }
  TRACE_EXIT();
  return ret;
}

STDMETHODIMP
CDeskBand::GetCommandString(UINT_PTR idCommand,
                            UINT uFlags,
                            LPUINT lpReserved,
                            LPSTR lpszName,
                            UINT uMaxNameLen)
{
  HRESULT  hr = E_INVALIDARG;
  return hr;
}

LRESULT CALLBACK
CDeskBand::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTER("CDeskBand::WndProc");
  LRESULT lResult = 0;
  CDeskBand  *pThis = (CDeskBand*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

  switch (uMessage)
    {
    case WM_NCCREATE:
      {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        pThis = (CDeskBand *)( lpcs->lpCreateParams );
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        //set the window handle
        pThis->m_hWnd = hWnd;
        SetTimer(hWnd, 0xdeadf00d, 3000, NULL);
        PaintHelper::Init();
      }
      break;

    case WM_ERASEBKGND:
	  if (pThis->m_CompositionEnabled)
	  {
		lResult = 1;
	  }
      break;

    case WM_COMMAND:
      pThis->OnCommand(wParam, lParam);
      break;

    case WM_TIMER:
      pThis->OnTimer(wParam, lParam);
      break;

    case WM_COPYDATA:
      pThis->OnCopyData((PCOPYDATASTRUCT) lParam);
      break;

    case WM_SETFOCUS:
      pThis->OnSetFocus();
      break;

    case WM_KILLFOCUS:
      pThis->OnKillFocus();
      break;

    case WM_SIZE:
      pThis->OnSize(lParam);
      break;

    case WM_WINDOWPOSCHANGING:
      pThis->OnWindowPosChanging(wParam, lParam);
      break;

    case WM_LBUTTONUP:
      SendMessage((HWND)LongToHandle(pThis->m_AppletMenu.command_window),
                  WM_USER + 1, 0, NULL);
      break;
    }

    if (uMessage != WM_ERASEBKGND)
    {
        lResult = DefWindowProc(hWnd, uMessage, wParam, lParam);
    }

    TRACE_EXIT();
    return lResult;
}

LRESULT
CDeskBand::OnCommand(WPARAM wParam, LPARAM lParam)
{
  return 0;
}

LRESULT
CDeskBand::OnTimer(WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTER_MSG("CDeskBand::OnTimer", wParam << " " << lParam);
  if (m_TimerBox != NULL)
    {
      if (m_LastCopyData == 0 || difftime(time(NULL), m_LastCopyData) > 2)
        {
          m_TimerBox->set_enabled(false);
          m_TimerBox->update(false);
        }
    }
  TRACE_EXIT();
  return 0;
}

LRESULT
CDeskBand::OnCopyData(PCOPYDATASTRUCT copy_data)
{
  TRACE_ENTER("CDeskBand::OnCopyData");
  m_LastCopyData = time(NULL);
  if (copy_data->dwData == APPLET_MESSAGE_MENU
      && copy_data->cbData == sizeof(AppletMenuData))
    {
      m_AppletMenu = *((AppletMenuData *) copy_data->lpData);
      m_HasAppletMenu = TRUE;
    }
  else if (m_TimerBox != NULL
           && copy_data->dwData == APPLET_MESSAGE_HEARTBEAT
           && copy_data->cbData == sizeof(AppletHeartbeatData))
    {
      AppletHeartbeatData *data = (AppletHeartbeatData *) copy_data->lpData;
      m_TimerBox->set_enabled(data->enabled);
      for (int s = 0; s < BREAK_ID_SIZEOF; s++)
        {
          m_TimerBox->set_slot(s, (BreakId) data->slots[s]);
        }
      for (int b = 0; b < BREAK_ID_SIZEOF; b++)
        {
          TimeBar *bar = m_TimerBox->get_time_bar(BreakId(b));
          if (bar != NULL)
            {
              bar->set_text(data->bar_text[b]);
              bar->set_bar_color((ITimeBar::ColorId) data->bar_primary_color[b]);
              bar->set_secondary_bar_color((ITimeBar::ColorId) data->bar_secondary_color[b]);
              bar->set_progress(data->bar_primary_val[b], data->bar_primary_max[b]);
              bar->set_secondary_progress(data->bar_secondary_val[b], data->bar_secondary_max[b]);
            }
        }

      m_TimerBox->update(false);
    }
  TRACE_EXIT();
  return 0;
}


LRESULT
CDeskBand::OnSize(LPARAM lParam)
{
  TRACE_ENTER_MSG("CDeskBand::OnSize",lParam);
  int   cx, cy;

  cx = LOWORD(lParam);
  cy = HIWORD(lParam);
  TRACE_MSG(cx << " " << cy);
  if (m_TimerBox != NULL)
    {
      m_TimerBox->set_size(cx, cy);
      m_TimerBox->update(true);
    }

  TRACE_EXIT();
  return 0;
}


void
CDeskBand::FocusChange(BOOL bFocus)
{
  TRACE_ENTER_MSG("CDeskBand::OnWindowPosChanging", bFocus);
  m_bFocus = bFocus;

  //inform the input object site that the focus has changed
  if (m_pSite)
    {
      m_pSite->OnFocusChangeIS((IDockingWindow*)this, bFocus);
    }
  TRACE_EXIT();
}


LRESULT
CDeskBand::OnSetFocus()
{
  TRACE_ENTER("CDeskBand::OnSetFocus");
  FocusChange(TRUE);
  TRACE_EXIT();
  return 0;
}


LRESULT
CDeskBand::OnKillFocus()
{
  TRACE_ENTER("CDeskBand::OnKillFocus");
  FocusChange(FALSE);
  TRACE_EXIT();
  return 0;
}

BOOL
CDeskBand::RegisterAndCreateWindow()
{
  TRACE_ENTER("CDeskBand::RegisterAndCreateWindow");
  //If the window doesn't exist yet, create it now.
  if (!m_hWnd)
    {
      //Can't create a child window without a parent.
      if (!m_hwndParent)
        {
          return FALSE;
        }

      //If the window class has not been registered, then do so.
      WNDCLASS wc;
      if (!GetClassInfo(g_hInst, DB_CLASS_NAME, &wc))
        {
          ZeroMemory(&wc, sizeof(wc));
          wc.style          = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
          wc.lpfnWndProc    = (WNDPROC)WndProc;
          wc.cbClsExtra     = 0;
          wc.cbWndExtra     = 0;
          wc.hInstance      = g_hInst;
          wc.hIcon          = NULL;
          wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
          wc.hbrBackground  = NULL;//(HBRUSH)(COLOR_WINDOWFRAME+1);
          wc.lpszMenuName   = NULL;
          wc.lpszClassName  = DB_CLASS_NAME;

          if (!RegisterClass(&wc))
            {
              //If RegisterClass fails, CreateWindow below will fail.
            }
        }

      RECT  rc;

      GetClientRect(m_hwndParent, &rc);

      //Create the window. The WndProc will set m_hWnd.
      HWND h = CreateWindowEx(WS_EX_TRANSPARENT,
                              DB_CLASS_NAME,
                              NULL,
                              WS_CHILD | WS_CLIPSIBLINGS,
                              rc.left,
                              rc.top,
                              rc.right - rc.left,
                              rc.bottom - rc.top,
                              m_hwndParent,
                              NULL,
                              g_hInst,
                              (LPVOID)this);
      
      m_TimerBox = new TimerBox(h, g_hInst, this);
    }

  return (NULL != m_hWnd);
  TRACE_EXIT();
}


LRESULT
CDeskBand::OnWindowPosChanging(WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTER_MSG("CDeskBand::OnWindowPosChanging", wParam << " " << lParam);
  if (m_TimerBox != NULL)
    {
      m_TimerBox->update(true);
    }
  TRACE_EXIT();
  return 0;
}
