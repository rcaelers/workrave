#include "PaintHelper.h"
#include "Debug.h"

#include <string>

using namespace std;

BEGIN_BUFFERED_PAINT PaintHelper::BeginBufferedPaint = NULL;
END_BUFFERED_PAINT PaintHelper::EndBufferedPaint = NULL;
BUFFERED_PAINT_SET_ALPHA PaintHelper::BufferedPaintSetAlpha = NULL;
BUFFERED_PAINT_UNINIT PaintHelper::BufferedPaintUnInit = NULL;
BUFFERED_PAINT_INIT PaintHelper::BufferedPaintInit = NULL;
DRAW_THEME_PARENT_BACKGROUND PaintHelper::DrawThemeParentBackground = NULL;
IS_THEME_ACTIVE PaintHelper::IsThemeActive = NULL;
DWM_EXTEND_FRAME_INTO_CLIENT_AREA PaintHelper::DwmExtendFrameIntoClientArea = NULL;
DWM_IS_COMPOSITION_ENABLED PaintHelper::DwmIsCompositionEnabled = NULL;
DWM_ENABLE_COMPOSITION PaintHelper::DwmEnableComposition = NULL;

bool PaintHelper::comp_enabled = false;

PaintHelper::PaintHelper(HWND hwnd)
{
    this->hwnd = hwnd; 
    MARGINS m = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &m);
}


PaintHelper::~PaintHelper()
{
}


HDC 
PaintHelper::BeginPaint()
{
  TRACE_ENTER("PaintHelper::BeginPaint");
     TRACE_MSG(hwnd);
      TRACE_MSG(&ps);
  hdc = ::BeginPaint(hwnd, &ps);
      TRACE_MSG("0a");
  paint_hdc = hdc;
      TRACE_MSG("0b");


  if (hdc)
    {
      TRACE_MSG("1");
      RECT rc;
      GetClientRect(hwnd, &rc);

      DrawThemeParentBackground(hwnd, hdc, &rc) ;

      if (comp_enabled)
        {
          TRACE_MSG("2");

          BP_PAINTPARAMS paint_params = {0};
          paint_params.cbSize = sizeof(paint_params);
          paint_buffer = BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, &paint_params, &paint_hdc);
        }
    }

  TRACE_EXIT();
  return paint_hdc;
}

void 
PaintHelper::EndPaint()
{
  TRACE_ENTER("PaintHelper::EndPaint");

  if (comp_enabled)
    {
      TRACE_MSG("1");
      BufferedPaintSetAlpha(paint_buffer, 0, 255);
      EndBufferedPaint(paint_buffer, TRUE);
    }
  ::EndPaint(hwnd, &ps);
 
  TRACE_EXIT();
}

void
PaintHelper::Init()
{
  TRACE_ENTER("PaintHelper::Init");
  HINSTANCE handle = LoadLibrary("UxTheme.dll");
  if (handle != NULL)
    {
      BeginBufferedPaint = (BEGIN_BUFFERED_PAINT)::GetProcAddress(handle, "BeginBufferedPaint");
      EndBufferedPaint = (END_BUFFERED_PAINT)::GetProcAddress(handle, "EndBufferedPaint");
      BufferedPaintSetAlpha = (BUFFERED_PAINT_SET_ALPHA)::GetProcAddress(handle, "BufferedPaintSetAlpha");
      BufferedPaintUnInit = (BUFFERED_PAINT_UNINIT)::GetProcAddress(handle, "BufferedPaintUnInit");
      BufferedPaintInit = (BUFFERED_PAINT_INIT)::GetProcAddress(handle, "BufferedPaintInit");

      DrawThemeParentBackground = (DRAW_THEME_PARENT_BACKGROUND)::GetProcAddress(handle,"DrawThemeParentBackground");
      IsThemeActive = (IS_THEME_ACTIVE)::GetProcAddress(handle,"IsThemeActive");
    }

  HINSTANCE dwm_handle = LoadLibrary("dwmapi.dll");
  if (dwm_handle != NULL)
    {
      DwmExtendFrameIntoClientArea = (DWM_EXTEND_FRAME_INTO_CLIENT_AREA)::GetProcAddress(dwm_handle, "DwmExtendFrameIntoClientArea");
      DwmIsCompositionEnabled = (DWM_IS_COMPOSITION_ENABLED)::GetProcAddress(dwm_handle, "DwmIsCompositionEnabled");
      DwmEnableComposition = (DWM_ENABLE_COMPOSITION)::GetProcAddress(dwm_handle, "DwmEnableComposition");
    }

  comp_enabled = false;
  TRACE_EXIT();
}

void 
PaintHelper::SetCompositionEnabled(bool enabled)
{
  TRACE_ENTER_MSG("PaintHelper::SetCompositionEnabled", enabled);
  comp_enabled = enabled;
  TRACE_EXIT();
}

