// Copyright (C) 2010 - 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_REST_HTTPERROR_HH
#define WORKRAVE_REST_HTTPERROR_HH

#include <string>
#include <exception>
#include <iostream>
#include <system_error>

namespace workrave
{
  namespace rest
  {
    enum class HttpErrorCode
    {
        // Http operation was successful.
        Success = 0,

        //
        Transport,
          
        // General failure.
        Failure,
    };

    class HttpError : public std::runtime_error
    {
    public:
      explicit HttpError(HttpErrorCode error_code, const std::string &detail = "")
        : std::runtime_error("HttpError"), error_code(error_code), detailed_information(detail)
      {
      }

      virtual ~HttpError() noexcept
      {
      }

      virtual const char *what() const noexcept
      {
        return detailed_information.c_str();
      }

      HttpErrorCode code() const
      {
        return error_code;
      }

    private:
      //
      HttpErrorCode error_code;
      std::string detailed_information;
    };
  }
}

#endif
