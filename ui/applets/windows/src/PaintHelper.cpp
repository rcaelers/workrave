#include "PaintHelper.h"
#include "Debug.h"

#include <uxtheme.h>
#include <gdiplus.h>

#include <string>

using namespace std;

BEGIN_BUFFERED_PAINT PaintHelper::BeginBufferedPaint = NULL;
END_BUFFERED_PAINT PaintHelper::EndBufferedPaint = NULL;
BUFFERED_PAINT_SET_ALPHA PaintHelper::BufferedPaintSetAlpha = NULL;
BUFFERED_PAINT_UNINIT PaintHelper::BufferedPaintUnInit = NULL;
BUFFERED_PAINT_INIT PaintHelper::BufferedPaintInit = NULL;
GET_BUFFERED_PAINT_BITS PaintHelper::GetBufferedPaintBits = NULL;

bool PaintHelper::composition_enabled = false;
bool PaintHelper::composition_available = false;

PaintHelper::PaintHelper(HWND hwnd)
{
  TRACE_ENTER_MSG("PaintHelper::PaintHelper", hwnd);
  this->hwnd = hwnd;
  TRACE_EXIT();
}

PaintHelper::~PaintHelper()
{
}

HDC
PaintHelper::BeginPaint()
{
  TRACE_ENTER("PaintHelper::BeginPaint");
  hdc = ::BeginPaint(hwnd, &ps);
  paint_hdc = hdc;

  if (hdc)
    {
      if (composition_enabled && composition_available)
        {
          RECT rc;
          BP_PAINTPARAMS paint_params = {0};

          GetClientRect(hwnd, &rc);

          paint_params.dwFlags = BPPF_ERASE;
          paint_params.cbSize = sizeof(paint_params);
          paint_buffer = BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, &paint_params, &paint_hdc);
          PatBlt(paint_hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, BLACKNESS);
          BufferedPaintSetAlpha(paint_buffer, 0, 255);
          alpha_set = false;
        }
      else
        {
          paint_buffer = NULL;
        }
    }

  TRACE_EXIT();
  return paint_hdc;
}

void
PaintHelper::EndPaint()
{
  TRACE_ENTER("PaintHelper::EndPaint");

  if (paint_buffer != NULL)
    {
      if (!alpha_set)
        {
          BufferedPaintSetAlpha(paint_buffer, 0, 255);
        }
      EndBufferedPaint(paint_buffer, TRUE);
      paint_buffer = NULL;
    }
  ::EndPaint(hwnd, &ps);

  TRACE_EXIT();
}

void
PaintHelper::FixIconAlpha(HICON icon)
{
  TRACE_ENTER("PaintHelper::FixIconAlpha");
  ICONINFO icon_info;
  RGBQUAD *paint_bits;
  Gdiplus::ARGB *paint_bits_argb;
  int row_size;
  HRESULT hr = (paint_buffer != NULL) ? S_OK : E_FAIL;

  if (SUCCEEDED(hr))
    {
      hr = GetBufferedPaintBits(paint_buffer, &paint_bits, &row_size);
    }

  if (SUCCEEDED(hr))
    {
      paint_bits_argb = (Gdiplus::ARGB *)paint_bits;
      if (!GetIconInfo(icon, &icon_info))
        {
          hr = E_FAIL;
        }
    }

  if (SUCCEEDED(hr))
    {
      HDC hdc = GetDC(0);
      BITMAPINFO bmi = {};

      bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);

      if (GetDIBits(hdc, icon_info.hbmColor, 0, 0, 0, &bmi, DIB_RGB_COLORS))
        {
          int icon_width = abs(bmi.bmiHeader.biWidth);
          int icon_height = abs(bmi.bmiHeader.biHeight);

          bmi.bmiHeader.biWidth = icon_width;
          bmi.bmiHeader.biHeight = -icon_height;
          bmi.bmiHeader.biCompression = BI_RGB;
          bmi.bmiHeader.biBitCount = 32;
          bmi.bmiHeader.biPlanes = 1;

          int row_delta = row_size - bmi.bmiHeader.biWidth;
          unsigned char *mask_data = (unsigned char *)malloc(icon_width * icon_height * 4);

          if (GetDIBits(hdc, icon_info.hbmMask, 0, abs(bmi.bmiHeader.biHeight), mask_data, &bmi, DIB_RGB_COLORS))
            {
              unsigned char *mask_ptr = mask_data;

              for (int y = 0; y < icon_height; y++)
                {
                  for (int x = 0; x < icon_width; x++)
                    {
                      if (mask_ptr[0])
                        {
                          *paint_bits_argb++ = 0x40000000;
                        }
                      else
                        {
                          *paint_bits_argb++ |= 0xff000000;
                        }
                      mask_ptr += 4;
                    }
                  paint_bits_argb += row_delta;
                }

              alpha_set = true;
            }

          free(mask_data);
        }

      ReleaseDC(0, hdc);
      DeleteObject(icon_info.hbmColor);
      DeleteObject(icon_info.hbmMask);
    }

  TRACE_EXIT();
}

void
PaintHelper::DrawIcon(int x, int y, HICON hIcon, int width, int height)
{
  TRACE_ENTER("PaintHelper::DrawIcon");
  DrawIconEx(paint_hdc, x, y, hIcon, width, height, 0, NULL, DI_NORMAL);
  FixIconAlpha(hIcon);
  TRACE_EXIT();
}

void
PaintHelper::Init()
{
  TRACE_ENTER("PaintHelper::Init");
  HINSTANCE handle = LoadLibrary("UxTheme.dll");

  composition_available = false;
  composition_enabled = false;

  if (handle != NULL)
    {
      BeginBufferedPaint = (BEGIN_BUFFERED_PAINT)::GetProcAddress(handle, "BeginBufferedPaint");
      EndBufferedPaint = (END_BUFFERED_PAINT)::GetProcAddress(handle, "EndBufferedPaint");
      BufferedPaintSetAlpha = (BUFFERED_PAINT_SET_ALPHA)::GetProcAddress(handle, "BufferedPaintSetAlpha");
      BufferedPaintUnInit = (BUFFERED_PAINT_UNINIT)::GetProcAddress(handle, "BufferedPaintUnInit");
      BufferedPaintInit = (BUFFERED_PAINT_INIT)::GetProcAddress(handle, "BufferedPaintInit");
      GetBufferedPaintBits = (GET_BUFFERED_PAINT_BITS)::GetProcAddress(handle, "GetBufferedPaintBits");

      if (BeginBufferedPaint != NULL && EndBufferedPaint != NULL && BufferedPaintSetAlpha != NULL && BufferedPaintUnInit != NULL
          && BufferedPaintInit != NULL && GetBufferedPaintBits != NULL)
        {
          TRACE_MSG("composition available");
          composition_available = true;
        }
    }

  TRACE_EXIT();
}

void
PaintHelper::SetCompositionEnabled(bool enabled)
{
  TRACE_ENTER_MSG("PaintHelper::SetCompositionEnabled", enabled);
  composition_enabled = enabled;
  TRACE_EXIT();
}

bool
PaintHelper::GetCompositionEnabled()
{
  return composition_enabled;
}
