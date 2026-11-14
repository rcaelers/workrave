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

#include "WindowsResources.hh"
#include "WindowsImaging.hh"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string_view>

using namespace workrave::windows;

namespace
{
  struct Checks
  {
    int failures{0};

    void require(bool condition, std::string_view message)
    {
      if (!condition)
        {
          std::cerr << message << '\n';
          ++failures;
        }
    }
  };

  void check_errors(Checks &checks)
  {
    check_hresult(S_OK, "Success");
    check_hresult(S_FALSE, "Success without a result");
    bool caught = false;
    try
      {
        check_hresult(E_FAIL, "Read image frame");
      }
    catch (const std::system_error &error)
      {
        caught = true;
        checks.require(error.code().value() == static_cast<int>(E_FAIL), "HRESULT exceptions must retain the error code");
        checks.require(std::string_view(error.what()).find("Read image frame") != std::string_view::npos,
                       "HRESULT exceptions must identify the failed operation");
      }
    checks.require(caught, "Failed HRESULTs must throw");

    SetLastError(ERROR_ACCESS_DENIED);
    try
      {
        check_win32(FALSE, "Native operation");
        checks.require(false, "Failed native calls must throw");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() == ERROR_ACCESS_DENIED, "Native exceptions must retain GetLastError");
      }
  }

  void check_bitmaps(Checks &checks)
  {
    auto dc = create_compatible_dc();
    auto *const original = GetCurrentObject(dc.get(), OBJ_BITMAP);
    HBITMAP bitmap_handle = nullptr;
    try
      {
        auto bitmap = create_bitmap(8, 8, 1, 1);
        bitmap_handle = bitmap.get();
        BITMAP details{};
        checks.require(GetObjectW(bitmap_handle, sizeof(details), &details) != 0, "The bitmap must be queryable while owned");
        GdiSelection selection(dc.get(), bitmap.get());
        checks.require(GetCurrentObject(dc.get(), OBJ_BITMAP) == bitmap.get(), "The test bitmap must be selected");
        check_hresult(E_FAIL, "Fail after selecting a bitmap");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() == static_cast<int>(E_FAIL), "Bitmap setup must succeed before testing unwinding");
      }
    checks.require(bitmap_handle != nullptr, "The bitmap test must allocate an owned resource");
    checks.require(GetCurrentObject(dc.get(), OBJ_BITMAP) == original, "Unwinding must restore the original GDI selection");
    BITMAP details{};
    checks.require(GetObjectW(bitmap_handle, sizeof(details), &details) == 0,
                   "Unwinding must delete the bitmap after deselecting it");
  }

  void check_dib_sections(Checks &checks)
  {
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = 2;
    info.bmiHeader.biHeight = -1;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    HBITMAP handle = nullptr;
    try
      {
        auto dib = create_dib_section(info);
        handle = dib.bitmap.get();
        DIBSECTION details{};
        checks.require(GetObjectW(handle, sizeof(details), &details) != 0, "The DIB factory must return a valid bitmap");
        checks.require(dib.pixels != nullptr && details.dsBm.bmBits == dib.pixels,
                       "The returned pixel buffer must belong to the owned bitmap");
        check_hresult(E_FAIL, "Fail after creating a DIB section");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() == static_cast<int>(E_FAIL), "DIB setup must succeed before testing unwinding");
      }
    BITMAP details{};
    checks.require(GetObjectW(handle, sizeof(details), &details) == 0, "Unwinding must release the DIB section");
  }

  void check_icons(Checks &checks)
  {
    HICON icon_handle = nullptr;
    auto *const shared_icon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));
    try
      {
        auto icon = copy_icon(shared_icon);
        icon_handle = icon.get();
        check_hresult(E_FAIL, "Fail after creating an icon");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() == static_cast<int>(E_FAIL), "Icon setup must succeed before testing unwinding");
      }
    checks.require(icon_handle != nullptr, "The icon test must allocate an owned resource");
    ICONINFO info{};
    const auto found = GetIconInfo(icon_handle, &info);
    UniqueBitmap color(info.hbmColor);
    UniqueBitmap mask(info.hbmMask);
    checks.require(found == FALSE, "Unwinding must destroy the owned icon");
  }

  void check_menus(Checks &checks)
  {
    HMENU parent_handle = nullptr;
    HMENU child_handle = nullptr;
    try
      {
        auto parent = create_popup_menu();
        parent_handle = parent.get();
        child_handle = append_submenu(parent.get(), create_popup_menu(), L"Test submenu");
        check_hresult(E_FAIL, "Fail after transferring submenu ownership");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() == static_cast<int>(E_FAIL), "Menu setup must succeed before testing unwinding");
      }
    checks.require(parent_handle != nullptr && child_handle != nullptr, "The menu test must transfer a valid submenu");
    checks.require(IsMenu(parent_handle) == FALSE && IsMenu(child_handle) == FALSE,
                   "Unwinding must destroy both the parent menu and its transferred submenu");

    auto orphan = create_popup_menu();
    child_handle = orphan.get();
    try
      {
        static_cast<void>(append_submenu(nullptr, std::move(orphan), L"Invalid parent"));
        checks.require(false, "Appending to an invalid menu must throw");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() > 0, "Menu failures must retain their Win32 error code");
      }
    checks.require(IsMenu(child_handle) == FALSE, "A failed append must release the unattached submenu");
  }

  void check_imaging(Checks &checks)
  {
    ComApartment apartment(COINIT_APARTMENTTHREADED);
    ImagingFactory factory;
    const auto path = std::filesystem::path(TEST_IMAGE_PATH).wstring();
    const auto decoded = factory.load(path);
    const auto [width, height] = decoded.size();
    checks.require(width > 0 && height > 0 && width <= 256 && height <= 256, "The image wrapper must return icon dimensions");
    if (width == 0 || height == 0 || width > 256 || height > 256)
      {
        return;
      }
    const auto converted = factory.to_pbgra(decoded);
    std::vector<BYTE> pixels(static_cast<size_t>(width) * height * 4);
    converted.copy_pixels(width * 4, pixels);
    checks.require(std::ranges::any_of(pixels, [](BYTE value) { return value != 0; }),
                   "The image wrapper must decode real pixels");
    try
      {
        converted.copy_pixels(width * 4, {});
        checks.require(false, "An insufficient image buffer must throw");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() < 0, "Image copy failures must preserve the HRESULT");
      }
    // A failed copy must leave the image usable and owned.
    converted.copy_pixels(width * 4, pixels);
    try
      {
        static_cast<void>(factory.load(path + L".missing"));
        checks.require(false, "A missing image must throw");
      }
    catch (const std::system_error &error)
      {
        checks.require(std::string_view(error.what()).find("Decode image") != std::string_view::npos,
                       "Image errors must identify the operation at the wrapper boundary");
      }
  }

  void check_apartments(Checks &checks)
  {
    APTTYPE type{};
    APTTYPEQUALIFIER qualifier{};
    try
      {
        ComApartment apartment(COINIT_MULTITHREADED);
        {
          // S_FALSE still adds a COM initialization reference that must be balanced.
          ComApartment nested(COINIT_MULTITHREADED);
        }
        {
          ComApartment borrowed(COINIT_APARTMENTTHREADED, ComApartment::ExistingApartment::AllowDifferentMode);
        }
        check_hresult(CoGetApartmentType(&type, &qualifier), "Read current apartment");
        checks.require(type == APTTYPE_MTA, "Borrowing a different apartment must not uninitialize its owner");
        try
          {
            ComApartment rejected(COINIT_APARTMENTTHREADED);
            checks.require(false, "Strict apartment initialization must reject a different mode");
          }
        catch (const std::system_error &error)
          {
            checks.require(error.code().value() == static_cast<int>(RPC_E_CHANGED_MODE),
                           "Apartment mode errors must be preserved");
          }
        check_hresult(E_FAIL, "Fail within COM scope");
      }
    catch (const std::system_error &error)
      {
        checks.require(error.code().value() == static_cast<int>(E_FAIL), "COM setup must succeed before testing unwinding");
      }
    const auto result = CoGetApartmentType(&type, &qualifier);
    checks.require(result == CO_E_NOTINITIALIZED, "Unwinding must balance both S_OK and S_FALSE COM initialization");
  }
} // namespace

int
main()
try
  {
    Checks checks;
    check_errors(checks);
    check_bitmaps(checks);
    check_dib_sections(checks);
    check_icons(checks);
    check_menus(checks);
    check_apartments(checks);
    check_imaging(checks);
    std::cout << "Windows resource checks: " << checks.failures << " failures\n";
    return checks.failures == 0 ? 0 : 1;
  }
catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
