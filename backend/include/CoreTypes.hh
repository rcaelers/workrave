// CoreTypes.hh --- The main controller interface
//
// Copyright (C) 2001 - 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_BACKEND_CORETYPES_HH
#define WORKRAVE_BACKEND_CORETYPES_HH

#ifdef __cplusplus
namespace workrave {
#endif
  
  /* Mode */
  typedef enum OperationMode
    {
      /* Breaks are reported to the user when due. */
      OPERATION_MODE_NORMAL=0,

      /* Monitoring is suspended. */
      OPERATION_MODE_SUSPENDED,

      /* Breaks are not reported to the user when due. */
      OPERATION_MODE_QUIET,

      /* Number of modes.*/
      OPERATION_MODE_SIZEOF
    } OperationMode;

  typedef enum UsageMode
    {
      /* Normal 'average' PC usage. */
      USAGE_MODE_NORMAL=0,

      /* User is reading. */
      USAGE_MODE_READING,

      /* Number of modes. */
      USAGE_MODE_SIZEOF
    }  UsageMode;

  //! ID of a break.
  enum BreakId
    {
      BREAK_ID_NONE = -1,
      BREAK_ID_MICRO_BREAK = 0,
      BREAK_ID_REST_BREAK,
      BREAK_ID_DAILY_LIMIT,
      BREAK_ID_SIZEOF
    };

  enum BreakHint
    {
      BREAK_HINT_NONE = 0,

      // Break was started on user request
      BREAK_HINT_USER_INITIATED = 1,

      // Natural break.
      BREAK_HINT_NATURAL_BREAK = 2
    };


  
#ifdef __cplusplus
}
#endif

#endif // WORKRAVE_BACKEND_CORETYPES_HH
