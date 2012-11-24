// Uri.hh --- General purpose string utility functions
//
// Copyright (C) 2010, 2011 Rob Caelers
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

#ifndef URI_HH
#define URI_HH

#include <string>

class Uri
{
public:
  static const std::string escape(const std::string &in);
  static const std::string unescape(const std::string &in);
};

#endif // URI_HH
