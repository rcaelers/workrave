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

#ifndef WORKRAVE_DBUS_DBUSEXCEPTION_HH
#define WORKRAVE_DBUS_DBUSEXCEPTION_HH

#include <boost/exception/all.hpp>
#include <iostream>

#include "utils/Exception.hh"

namespace workrave
{
  namespace dbus
  {
    extern const char *DBUS_ERROR_FAILED;
    extern const char *DBUS_ERROR_NOT_SUPPORTED;
    extern const char *DBUS_ERROR_INVALID_ARGS;
    extern const char *DBUS_ERROR_UNKNOWN_METHOD;

    class DBusException : public workrave::utils::Exception
    {
    public:
      explicit DBusException(const std::string &detail)
        : Exception(detail)
      {
      }

      ~DBusException() override = default;
    };

    using message_info = boost::error_info<struct tag_message_info, std::string>;
    using error_code_info = boost::error_info<struct tag_error_code_info, std::string>;
    using object_info = boost::error_info<struct tag_oject_info, std::string>;
    using interface_info = boost::error_info<struct tag_interface_info, std::string>;
    using method_info = boost::error_info<struct tag_method_info, std::string>;
    using argument_info = boost::error_info<struct tag_argument_info, std::string>;
    using actual_type_info = boost::error_info<struct tag_type_info, std::string>;
    using expected_type_info = boost::error_info<struct tag_expected_type_info, std::string>;
    using field_info = boost::error_info<struct tag_field_info, std::string>;
    using parameter_info = boost::error_info<struct tag_parameter_info, std::string>;
    using field_path_info = boost::error_info<struct tag_field_path_info, std::string>;

    class DBusRemoteException
      : public virtual boost::exception
      , public virtual std::exception
    {
    public:
      std::string error()
      {
        std::string ret;
        if (const std::string *msg = boost::get_error_info<error_code_info>(*this))
          {
            ret = *msg;
          }
        return ret;
      }

      DBusRemoteException &operator<<(const field_info &rhs)
      {
        if (const std::string *msg = boost::get_error_info<field_path_info>(*this))
          {
            *this << field_path_info(rhs.value() + "." + *msg);
          }
        else
          {
            *this << field_path_info(rhs.value());
          }

        return *this;
      }

      void prepend_field(const std::string &field)
      {
        if (const std::string *msg = boost::get_error_info<field_path_info>(*this))
          {
            *this << field_path_info(field + "." + *msg);
          }
        else
          {
            *this << field_path_info(field);
          }
      }

      std::string diag()
      {
        std::string ret;

        if (const std::string *msg = boost::get_error_info<message_info>(*this))
          {
            ret += *msg;
          }
        if (const std::string *msg = boost::get_error_info<object_info>(*this))
          {
            ret += " object path=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<interface_info>(*this))
          {
            ret += " interface=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<method_info>(*this))
          {
            ret += " method=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<argument_info>(*this))
          {
            ret += " argument=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<actual_type_info>(*this))
          {
            ret += " type=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<expected_type_info>(*this))
          {
            ret += " expected_type=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<field_path_info>(*this))
          {
            ret += " fieldpath=" + *msg;
          }
        if (const std::string *msg = boost::get_error_info<parameter_info>(*this))
          {
            ret += " parameter=" + *msg;
          }

        return ret;
      }
    };
  } // namespace dbus
} // namespace workrave

#endif // WORKRAVE_DBUS_DBUSEXCEPTION_HH
