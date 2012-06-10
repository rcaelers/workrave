// CoreTypes.hh --- The main controller interface
//
// Copyright (C) 2001 - 2009, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef CORETYPES_HH
#define CORETYPES_HH

#include "enum.h"

namespace workrave {

  //! ID of a break.
  enum BreakId
    {
      BREAK_ID_NONE = -1,
      BREAK_ID_MICRO_BREAK = 0,
      BREAK_ID_REST_BREAK,
      BREAK_ID_DAILY_LIMIT,
      BREAK_ID_SIZEOF
    };
};

#endif // CORETYPES_HH
