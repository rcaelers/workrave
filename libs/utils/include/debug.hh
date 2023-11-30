// Copyright (C) 2001 - 2010, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_DEBUG_HH
#define WORKRAVE_UTILS_DEBUG_HH

#include <cassert>
#include <boost/current_function.hpp>

#if !defined(HAVE_TRACING)

#  define TRACE_ENTRY(...)
#  define TRACE_ENTRY_MSG(...)
#  define TRACE_ENTRY_PAR(...)
#  define TRACE_MSG(...)
#  define TRACE_VAR(...)
#else

#  include <spdlog/spdlog.h>
#  include <spdlog/fmt/ostr.h>

#  include <boost/noncopyable.hpp>
#  include <string_view>
#  include <array>

constexpr std::string_view
prettify_function(std::string_view func)
{
  auto start_of_args = func.find_first_of("(");
  if (start_of_args == std::string_view::npos)
    {
      start_of_args = func.length();
    }

  auto end_of_ret_type = func.find_last_of(" ", start_of_args);
  return func.substr(end_of_ret_type + 1, start_of_args - end_of_ret_type - 1);
}

struct ScopedTraceAutoFmt
{
};

// https ://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time
template<size_t N>
struct gen_fmt
{
  static constexpr auto impl() noexcept
  {
    constexpr auto str = std::string_view{"{} "};
    constexpr size_t len = str.size() * N;
    std::array<char, len + 1> arr{};
    auto append = [i = 0, &arr](auto const &s) mutable {
      for (auto c: s)
        arr[i++] = c;
    };
    for (size_t j = 0; j < N; j++)
      append(str);
    return arr;
  }
  static constexpr auto arr = impl();
  static constexpr std::string_view value{arr.data(), arr.size() - 2};
};

#  if FMT_VERSION < 80000

namespace fmt
{
  inline auto runtime(std::string_view s) -> std::string_view
  {
    return s;
  }
} // namespace fmt

#  endif

class ScopedTrace : public boost::noncopyable
{
public:
  ScopedTrace(const std::string &func)
    : func(func)
  {
    if (logger)
      {
        logger->trace(fmt::runtime("> " + func));
      }
  }

  template<class... Param>
  ScopedTrace(const std::string &func, const std::string &fmt, const Param &...p)
    : func(func)
  {
    if (logger)
      {
        logger->trace(fmt::runtime("> " + func + " " + fmt), p...);
      }
  }
  template<class... Param>
  ScopedTrace(const ScopedTraceAutoFmt &, const std::string &func, const Param &...p)
    : func(func)
  {
    constexpr std::string_view fmt = gen_fmt<sizeof...(Param)>::value;

    if (logger)
      {
        logger->trace(fmt::runtime("> " + func + " " + std::string{fmt}), p...);
      }
  }

  ~ScopedTrace()
  {
    if (logger)
      {
        logger->trace("< " + func);
      }
  }

  template<class... Param>
  void msg(const std::string &fmt, const Param &...p)
  {
    if (logger)
      {
        logger->trace(fmt::runtime("= " + func + " " + fmt), p...);
      }
  }

  template<class... Param>
  void var(const Param &...p)
  {
    if (logger)
      {
        constexpr std::string_view fmt = gen_fmt<sizeof...(Param)>::value;
        logger->trace(fmt::runtime("= " + func + " " + std::string{fmt}), p...);
      }
  }

  static void init(std::shared_ptr<spdlog::logger> logger)
  {
    ScopedTrace::logger = logger;
  }

private:
  std::string func;
  static std::shared_ptr<spdlog::logger> logger;
};

#  define TRACE_ENTRY(...) ScopedTrace trace_(std::string{prettify_function(static_cast<const char *>(BOOST_CURRENT_FUNCTION))})
#  define TRACE_ENTRY_MSG(...) \
    ScopedTrace trace_(std::string{prettify_function(static_cast<const char *>(BOOST_CURRENT_FUNCTION))}, __VA_ARGS__)
#  define TRACE_ENTRY_PAR(...)                                                                            \
    ScopedTrace trace_(ScopedTraceAutoFmt{},                                                              \
                       std::string{prettify_function(static_cast<const char *>(BOOST_CURRENT_FUNCTION))}, \
                       __VA_ARGS__)

#  define TRACE_VAR(...) trace_.var(__VA_ARGS__)
#  define TRACE_MSG(...) trace_.msg(__VA_ARGS__)

#endif // HAVE_TRACING

#endif // WORKRAVE_UTILS_DEBUG_HH
