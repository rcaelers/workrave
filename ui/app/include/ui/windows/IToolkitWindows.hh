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

#ifndef ITOOLKIT_WINDOWS_HH
#define ITOOLKIT_WINDOWS_HH

#include <memory>
#include <boost/signals2.hpp>

#include <windows.h>

class IToolkitWindows
{
public:
  virtual ~IToolkitWindows() = default;

  struct event_combiner
  {
    using result_type = bool;

    template<typename InputIterator>
    bool operator()(InputIterator first, InputIterator last) const
    {
      return std::all_of(first, last, [](bool b) { return b; });
    }
  };

  virtual boost::signals2::signal<bool(MSG *msg), event_combiner> &hook_event() = 0;
  virtual HWND get_event_hwnd() const = 0;
};

#endif // ITOOLKIT_WINDOWS_HH
