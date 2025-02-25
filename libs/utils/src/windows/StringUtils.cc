// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "utils/StringUtils.hh"

#include <windows.h>

std::string
workrave::utils::utf16_to_utf8(const std::wstring &s)
{
  std::string ret;
  int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.length(), nullptr, 0, nullptr, nullptr);
  if (len > 0)
    {
      ret.resize(len);
      WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.length(), ret.data(), len, nullptr, nullptr);
    }
  return ret;
}

std::wstring
workrave::utils::utf8_to_utf16(const std::string &s)
{
  std::wstring ret;
  int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), nullptr, 0);
  if (len > 0)
    {
      ret.resize(len);
      MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), ret.data(), len);
    }
  return ret;
}
