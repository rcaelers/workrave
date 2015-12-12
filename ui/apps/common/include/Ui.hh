// Copyright (C) 2014, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_APPS_COMMON_UI_HH
#define WORKRAVE_APPS_COMMON_UI_HH

#include <string>

#include "core/CoreTypes.hh"
#include "UiTypes.hh"

namespace workrave
{
  namespace ui
  {
    class Ui
    {
    public:
      static const std::string get_break_name(workrave::BreakId id);
      static const std::string get_break_icon_filename(workrave::BreakId id);
      static const std::string get_status_icon_filename(StatusIconType id);
    };
  }
}

#endif // WORKRAVE_APPS_COMMON_UI_HH
