// TimeBar.h --- Time bar
//
// Copyright (C) 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef PAINTHELPER_H
#define PAINTHELPER_H

#define TMSCHEMA_H
#include <windows.h>
#include <Uxtheme.h>
#include <vssym32.h>

typedef HRESULT (__stdcall *DWM_EXTEND_FRAME_INTO_CLIENT_AREA)(HWND ,const MARGINS* );
typedef HRESULT (__stdcall *DWM_IS_COMPOSITION_ENABLED)(BOOL *pfEnabled);
typedef HRESULT (__stdcall *DWM_ENABLE_COMPOSITION)(UINT uCompositionAction);

typedef HRESULT (__stdcall *BUFFERED_PAINT_INIT)(VOID);
typedef HRESULT (__stdcall *BUFFERED_PAINT_UNINIT)(VOID);
typedef HPAINTBUFFER (__stdcall *BEGIN_BUFFERED_PAINT)(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams,  HDC *phdc);
typedef HRESULT (__stdcall *END_BUFFERED_PAINT)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
typedef HRESULT (__stdcall *BUFFERED_PAINT_SET_ALPHA)(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);
typedef HRESULT (__stdcall *DRAW_THEME_PARENT_BACKGROUND)(HWND hwnd, HDC hdc, const RECT *prc);
typedef BOOL (__stdcall *IS_THEME_ACTIVE)(void) ;
typedef HRESULT (__stdcall *DRAW_THEME_PARENT_BACKGROUND)(HWND, HDC, const RECT *);


class PaintHelper
{
public:
  PaintHelper(HWND hwnd);
  ~PaintHelper();

  HDC BeginPaint();
  void EndPaint();

  static void SetCompositionEnabled(bool enabled);
  static void Init();

private:
  HPAINTBUFFER paint_buffer;
  PAINTSTRUCT ps;
  HWND hwnd;
  HDC hdc;
  HDC paint_hdc;

  static bool comp_enabled;

  static BUFFERED_PAINT_UNINIT BufferedPaintUnInit;
  static BUFFERED_PAINT_INIT BufferedPaintInit;
  static BEGIN_BUFFERED_PAINT BeginBufferedPaint;
  static END_BUFFERED_PAINT EndBufferedPaint;
  static BUFFERED_PAINT_SET_ALPHA BufferedPaintSetAlpha;
  static DRAW_THEME_PARENT_BACKGROUND DrawThemeParentBackground;
  static IS_THEME_ACTIVE IsThemeActive;

  static DWM_EXTEND_FRAME_INTO_CLIENT_AREA DwmExtendFrameIntoClientArea;
  static DWM_IS_COMPOSITION_ENABLED DwmIsCompositionEnabled;
  static DWM_ENABLE_COMPOSITION DwmEnableComposition;
};

#endif // PAINTHELPER_H

