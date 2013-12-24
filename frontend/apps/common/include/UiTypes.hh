//
// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef UITYPES_HH
#define UITYPES_HH

typedef int BreakFlags;
  
const static int BREAK_FLAGS_NONE            = 0;
const static int BREAK_FLAGS_POSTPONABLE     = 1 << 0;
const static int BREAK_FLAGS_SKIPPABLE       = 1 << 1;
const static int BREAK_FLAGS_NO_EXERCISES    = 1 << 2;
const static int BREAK_FLAGS_NATURAL         = 1 << 3;
const static int BREAK_FLAGS_USER_INITIATED  = 1 << 4;

#endif // UITYPES_HH
