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

#ifndef WORKRAVE_REST_ERROR_HH
#define WORKRAVE_REST_ERROR_HH

#include <string>
#include <exception>
#include <iostream>
#include <system_error>

namespace workrave
{
  namespace rest
  {
    enum class AuthErrorCode
    {
        // OAuth initialization was successful.
        Success = 0,

        // Already performing a OAuth initialization.
        Busy,

        // System error.
        System,

        // OAuth protocol violation.
        Protocol,

        // OAuth server reported error.
        Server,

        // General failure.
        Failed,
    };

    class AuthError : public std::runtime_error
    {
    public:
      explicit AuthError(AuthErrorCode error_code, const std::string &detail = "")
        : std::runtime_error("AuthError"), error_code(error_code), detailed_information(detail)
      {
      }

      virtual ~AuthError() noexcept
      {
      }

      virtual const char *what() const noexcept
      {
        return detailed_information.c_str();
      }

      AuthErrorCode code() const
      {
        return error_code;
      }

    private:
      //
      AuthErrorCode error_code;
      std::string detailed_information;
    };
  }
}

#endif
