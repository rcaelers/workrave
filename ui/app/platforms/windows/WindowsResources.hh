// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_WINDOWS_RESOURCES_HH
#define WORKRAVE_WINDOWS_RESOURCES_HH

#include <windows.h>
#include <objbase.h>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace workrave::windows
{
  inline void check_hresult(HRESULT result, std::string_view operation)
  {
    if (FAILED(result))
      {
        throw std::system_error(static_cast<int>(result), std::system_category(), std::string(operation));
      }
  }

  [[noreturn]] inline void throw_last_error(std::string_view operation)
  {
    const auto error = GetLastError();
    const auto code = error == ERROR_SUCCESS ? ERROR_GEN_FAILURE : error;
    throw std::system_error(static_cast<int>(code), std::system_category(), std::string(operation));
  }

  inline void check_win32(BOOL result, std::string_view operation)
  {
    if (result == FALSE)
      {
        throw_last_error(operation);
      }
  }

  // For Win32 resource factories whose failure value is null.
  template<typename Handle>
  Handle check_handle(Handle handle, std::string_view operation)
  {
    if (handle == nullptr)
      {
        throw_last_error(operation);
      }
    return handle;
  }

  template<auto Release>
  struct HandleDeleter
  {
    template<typename Handle>
    void operator()(Handle *handle) const noexcept
    {
      Release(handle);
    }
  };

  template<typename Handle, auto Release>
  using UniqueHandle = std::unique_ptr<std::remove_pointer_t<Handle>, HandleDeleter<Release>>;

  using UniqueBitmap = UniqueHandle<HBITMAP, DeleteObject>;
  using UniqueBrush = UniqueHandle<HBRUSH, DeleteObject>;
  using UniqueFont = UniqueHandle<HFONT, DeleteObject>;
  using UniqueIcon = UniqueHandle<HICON, DestroyIcon>;
  using UniqueDC = UniqueHandle<HDC, DeleteDC>;
  using UniqueMenu = UniqueHandle<HMENU, DestroyMenu>;

  inline UniqueDC create_compatible_dc(HDC source = nullptr)
  {
    return UniqueDC(check_handle(CreateCompatibleDC(source), "Create compatible drawing context"));
  }

  inline UniqueBitmap create_compatible_bitmap(HDC source, int width, int height)
  {
    return UniqueBitmap(check_handle(CreateCompatibleBitmap(source, width, height), "Create compatible bitmap"));
  }

  inline UniqueBitmap create_bitmap(int width, int height, UINT planes, UINT bits_per_pixel, const void *bits = nullptr)
  {
    return UniqueBitmap(check_handle(CreateBitmap(width, height, planes, bits_per_pixel, bits), "Create bitmap"));
  }

  struct DibSection
  {
    UniqueBitmap bitmap;
    void *pixels{nullptr};
  };

  inline DibSection create_dib_section(const BITMAPINFO &info,
                                       HDC dc = nullptr,
                                       UINT usage = DIB_RGB_COLORS,
                                       HANDLE section = nullptr,
                                       DWORD offset = 0)
  {
    DibSection result;
    result.bitmap.reset(check_handle(CreateDIBSection(dc, &info, usage, &result.pixels, section, offset), "Create DIB section"));
    return result;
  }

  inline UniqueBrush create_solid_brush(COLORREF color)
  {
    return UniqueBrush(check_handle(CreateSolidBrush(color), "Create solid brush"));
  }

  inline UniqueFont create_font(const LOGFONTW &description)
  {
    return UniqueFont(check_handle(CreateFontIndirectW(&description), "Create font"));
  }

  inline UniqueIcon create_icon(ICONINFO info)
  {
    return UniqueIcon(check_handle(CreateIconIndirect(&info), "Create icon"));
  }

  inline UniqueIcon copy_icon(HICON source)
  {
    return UniqueIcon(check_handle(CopyIcon(source), "Copy icon"));
  }

  inline UniqueMenu create_popup_menu()
  {
    return UniqueMenu(check_handle(CreatePopupMenu(), "Create popup menu"));
  }

  // On success the parent takes ownership; the returned handle is borrowed.
  inline HMENU append_submenu(HMENU parent, UniqueMenu child, const std::wstring &label)
  {
    // Win32 stores submenu handles in an integer-sized field.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto id = reinterpret_cast<UINT_PTR>(child.get());
    check_win32(AppendMenuW(parent, MF_POPUP, id, label.c_str()), "Append submenu");
    return child.release();
  }

  inline void append_menu_item(HMENU menu, UINT flags, UINT_PTR id, const std::wstring &label)
  {
    check_win32(AppendMenuW(menu, flags, id, label.c_str()), "Append menu item");
  }

  inline void append_menu_separator(HMENU menu)
  {
    check_win32(AppendMenuW(menu, MF_SEPARATOR, 0, nullptr), "Append menu separator");
  }

  inline RECT get_window_rect(HWND window)
  {
    RECT result{};
    check_win32(GetWindowRect(window, &result), "Read window bounds");
    return result;
  }

  class ComApartment
  {
  public:
    enum class ExistingApartment
    {
      RequireRequestedMode,
      AllowDifferentMode,
    };

    explicit ComApartment(DWORD mode, ExistingApartment existing = ExistingApartment::RequireRequestedMode)
    {
      const auto result = CoInitializeEx(nullptr, mode);
      if (result == RPC_E_CHANGED_MODE && existing == ExistingApartment::AllowDifferentMode)
        {
          return;
        }
      check_hresult(result, "Initialize COM apartment");
      initialized = true;
    }

    ~ComApartment()
    {
      if (initialized)
        {
          CoUninitialize();
        }
    }
    ComApartment(const ComApartment &) = delete;
    ComApartment &operator=(const ComApartment &) = delete;
    ComApartment(ComApartment &&) = delete;
    ComApartment &operator=(ComApartment &&) = delete;

  private:
    bool initialized{false};
  };

  // Restore selected objects before their owners delete them, including on error.
  class GdiSelection
  {
  public:
    GdiSelection(HDC dc, HGDIOBJ object)
      : dc(dc)
      , previous(SelectObject(dc, object))
    {
      if (previous == nullptr || previous == HGDI_ERROR)
        {
          throw_last_error("Select GDI object");
        }
    }
    ~GdiSelection()
    {
      SelectObject(dc, previous);
    }
    GdiSelection(const GdiSelection &) = delete;
    GdiSelection &operator=(const GdiSelection &) = delete;
    GdiSelection(GdiSelection &&) = delete;
    GdiSelection &operator=(GdiSelection &&) = delete;

  private:
    HDC dc;
    HGDIOBJ previous;
  };

  class PaintScope
  {
  public:
    explicit PaintScope(HWND window)
      : window(window)
    {
      check_handle(BeginPaint(window, &paint), "Begin painting");
    }
    ~PaintScope()
    {
      EndPaint(window, &paint);
    }
    PaintScope(const PaintScope &) = delete;
    PaintScope &operator=(const PaintScope &) = delete;
    PaintScope(PaintScope &&) = delete;
    PaintScope &operator=(PaintScope &&) = delete;

    HDC get() const
    {
      return paint.hdc;
    }

  private:
    HWND window;
    PAINTSTRUCT paint{};
  };
} // namespace workrave::windows

#endif
