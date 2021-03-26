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
#include <uxtheme.h>
#include <vssym32.h>

typedef HRESULT(__stdcall *BUFFERED_PAINT_INIT)(VOID);
typedef HRESULT(__stdcall *BUFFERED_PAINT_UNINIT)(VOID);
typedef HPAINTBUFFER(__stdcall *BEGIN_BUFFERED_PAINT)(HDC hdcTarget,
                                                      const RECT *prcTarget,
                                                      BP_BUFFERFORMAT dwFormat,
                                                      BP_PAINTPARAMS *pPaintParams,
                                                      HDC *phdc);
typedef HRESULT(__stdcall *END_BUFFERED_PAINT)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
typedef HRESULT(__stdcall *BUFFERED_PAINT_SET_ALPHA)(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);
typedef HRESULT(__stdcall *GET_BUFFERED_PAINT_BITS)(HPAINTBUFFER hBufferedPaint, RGBQUAD **ppbBuffer, int *pcxRow);

class PaintHelper
{
public:
  PaintHelper(HWND hwnd);
  ~PaintHelper();

  HDC BeginPaint();
  void EndPaint();

  void DrawIcon(int x, int y, HICON icon, int width, int height);
  void FixIconAlpha(HICON icon);

  static void SetCompositionEnabled(bool enabled);
  static bool GetCompositionEnabled();
  static void Init();

private:
  HPAINTBUFFER paint_buffer{nullptr};
  PAINTSTRUCT ps{};
  HWND hwnd{};
  HDC hdc{};
  HDC paint_hdc{};
  bool alpha_set{false};

  static bool composition_enabled;
  static bool composition_available;

public:
  static BUFFERED_PAINT_UNINIT BufferedPaintUnInit;
  static BUFFERED_PAINT_INIT BufferedPaintInit;
  static BEGIN_BUFFERED_PAINT BeginBufferedPaint;
  static END_BUFFERED_PAINT EndBufferedPaint;
  static BUFFERED_PAINT_SET_ALPHA BufferedPaintSetAlpha;
  static GET_BUFFERED_PAINT_BITS GetBufferedPaintBits;
};

#endif // PAINTHELPER_H
