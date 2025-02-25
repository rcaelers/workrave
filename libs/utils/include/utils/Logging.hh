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

#ifndef WORKAVE_LIBS_UTILS_LOGGING_HH
#define WORKAVE_LIBS_UTILS_LOGGING_HH

#include <string>
#include <atomic>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace workrave::utils
{
  class Logging
  {
  public:
    static std::shared_ptr<spdlog::logger> create(std::string domain);
  };
} // namespace workrave::utils

#if FMT_VERSION >= 90000

template<typename T>
struct fmt::formatter<std::atomic<T>> : fmt::formatter<T>
{
  auto format(const typename std::atomic<T> &x, format_context &ctx) const
  {
    return formatter<T>::format(x.load(), ctx);
  }
};

#endif

#if FMT_VERSION < 80000

namespace fmt
{
  inline auto runtime(std::string_view s) -> std::string_view
  {
    return s;
  }
} // namespace fmt

#endif


#endif // WORKAVE_LIBS_UTILS_LOGGING_HH
