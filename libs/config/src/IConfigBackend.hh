// Copyright (C) 2001 - 2007, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef ICONFIGBACKEND_HH
#define ICONFIGBACKEND_HH

#include <string>

namespace workrave
{
  namespace config
  {
    class IConfiguratorListener;
  }
} // namespace workrave

#include "Variant.hh"

class IConfigBackend
{
public:
  virtual ~IConfigBackend() = default;

  virtual bool load(std::string filename) = 0;
  virtual bool save(std::string filename) = 0;
  virtual bool save() = 0;

  virtual bool remove_key(const std::string &key) = 0;
  virtual bool has_user_value(const std::string &key) = 0;
  virtual bool get_value(const std::string &key, VariantType type, Variant &value) const = 0;
  virtual bool set_value(const std::string &key, Variant &value) = 0;
};

class IConfigBackendMonitoring
{
public:
  virtual ~IConfigBackendMonitoring() = default;

  virtual void set_listener(workrave::config::IConfiguratorListener *listener) = 0;
  virtual bool add_listener(const std::string &key_prefix) = 0;
  virtual bool remove_listener(const std::string &key_prefix) = 0;
};

#endif // ICONFIGBACKEND_HH
