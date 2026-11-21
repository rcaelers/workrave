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

#ifndef WORKRAVE_WINDOWS_IMAGING_HH
#define WORKRAVE_WINDOWS_IMAGING_HH

#include "WindowsResources.hh"

#include <wincodec.h>
#include <wrl/client.h>
#include <span>
#include <stdexcept>
#include <utility>

namespace workrave::windows
{
  class ImagingFactory;

  // An owned WIC bitmap source. Pixel format conversion is explicit.
  class Bitmap
  {
  public:
    struct Size
    {
      UINT width{0};
      UINT height{0};
    };

    Size size() const
    {
      Size result;
      check_hresult(source->GetSize(&result.width, &result.height), "Read image size");
      return result;
    }

    void copy_pixels(UINT stride, std::span<BYTE> destination) const
    {
      if (!std::in_range<UINT>(destination.size()))
        {
          throw std::length_error("Image pixel buffer exceeds the WIC size limit");
        }
      check_hresult(source->CopyPixels(nullptr, stride, static_cast<UINT>(destination.size()), destination.data()),
                    "Copy image pixels");
    }

  private:
    friend class ImagingFactory;
    explicit Bitmap(Microsoft::WRL::ComPtr<IWICBitmapSource> source)
      : source(std::move(source))
    {
    }

    Microsoft::WRL::ComPtr<IWICBitmapSource> source;
  };

  // These wrappers require a COM apartment that outlives their instances.
  class ImagingFactory
  {
  public:
    ImagingFactory()
    {
      check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)),
                    "Create imaging factory");
    }

    Bitmap load(const std::wstring &path) const
    {
      Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
      Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
      check_hresult(
        factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder),
        "Decode image");
      check_hresult(decoder->GetFrame(0, &frame), "Read image frame");
      return Bitmap(std::move(frame));
    }

    Bitmap to_pbgra(const Bitmap &image) const
    {
      Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
      check_hresult(factory->CreateFormatConverter(&converter), "Create image converter");
      check_hresult(converter->Initialize(image.source.Get(),
                                          GUID_WICPixelFormat32bppPBGRA,
                                          WICBitmapDitherTypeNone,
                                          nullptr,
                                          0,
                                          WICBitmapPaletteTypeCustom),
                    "Convert image format");
      return Bitmap(std::move(converter));
    }

  private:
    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
  };
} // namespace workrave::windows

#endif
