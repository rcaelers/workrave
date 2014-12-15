// Exception.hh --- Base exception
//
// Copyright (C) 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_EXCEPTION_HH
#define WORKRAVE_UTILS_EXCEPTION_HH


#include <string>
#include <exception>

namespace workrave
{
  namespace utils
  {
    class Exception : public std::exception
    {
    public:
      explicit Exception(const std::string &detail)
        : detailed_information(detail)
      {
      }
      
      explicit Exception(const Exception &parent, const std::string &detail)
      {
        detailed_information = parent.details() + ", " + detail;
      }
      
      // FIXME: implement what
      virtual std::string details() const
      {
        return detailed_information;
      }
      
    private:
      //
      std::string detailed_information;
    };
  }
}

#endif // WORKRAVE_UTILS_EXCEPTION_HH
