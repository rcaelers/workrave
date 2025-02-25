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

#ifndef WORKAVE_LIBS_UTILS_STRINGUTILS_HH
#define WORKAVE_LIBS_UTILS_STRINGUTILS_HH

#include <string>

namespace workrave::utils
{
  std::string utf16_to_utf8(const std::wstring &s);
  std::wstring utf8_to_utf16(const std::string &s);
} // namespace workrave::utils

#endif // WORKAVE_LIBS_UTILS_STRINGUTILS_HH
