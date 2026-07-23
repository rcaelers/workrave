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

#include "rpc/Duration.hh"

#include <cctype>
#include <charconv>
#include <string>

#include "rpc/RpcException.hh"

namespace rpc
{
  std::chrono::seconds parse_duration(std::string_view text)
  {
    std::chrono::seconds total{0};
    std::size_t i = 0;
    bool any_token = false;

    while (i < text.size())
      {
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i])) != 0)
          {
            ++i;
          }
        if (i >= text.size())
          {
            break;
          }

        std::size_t start = i;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])) != 0)
          {
            ++i;
          }
        if (i == start)
          {
            throw RpcException("invalid duration '" + std::string(text) + "': expected a number");
          }

        long long value = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr, ec] = std::from_chars(text.data() + start, text.data() + i, value);
        if (ec != std::errc())
          {
            throw RpcException("invalid duration '" + std::string(text) + "': number out of range");
          }
        any_token = true;

        if (i < text.size() && std::isspace(static_cast<unsigned char>(text[i])) == 0)
          {
            char unit = static_cast<char>(std::tolower(static_cast<unsigned char>(text[i])));
            ++i;
            switch (unit)
              {
              case 'h':
                total += std::chrono::hours(value);
                break;
              case 'm':
                total += std::chrono::minutes(value);
                break;
              case 's':
                total += std::chrono::seconds(value);
                break;
              default:
                throw RpcException("invalid duration '" + std::string(text) + "': unknown unit '" + unit + "'");
              }
          }
        else
          {
            // A bare number with no unit is treated as minutes.
            total += std::chrono::minutes(value);
          }
      }

    if (!any_token)
      {
        throw RpcException("invalid duration '" + std::string(text) + "': empty");
      }
    return total;
  }
} // namespace rpc
