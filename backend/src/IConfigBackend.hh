// IConfigBackend.hh
//
// Copyright (C) 2001 - 2007 Rob Caelers <robc@krandor.nl>
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
  class IConfiguratorListener;
}

using namespace workrave;

#include "Variant.hh"

class IConfigBackend
{
public:
  virtual ~IConfigBackend() {}

  virtual bool load(std::string filename) = 0;
  virtual bool save(std::string filename) = 0;
  virtual bool save()                     = 0;

  virtual bool remove_key(const std::string &key)                                        = 0;
  virtual bool get_value(const std::string &key, VariantType type, Variant &value) const = 0;
  virtual bool set_value(const std::string &key, Variant &value)                         = 0;
};

class IConfigBackendMonitoring
{
public:
  virtual ~IConfigBackendMonitoring() {}

  virtual void set_listener(IConfiguratorListener *listener)  = 0;
  virtual bool add_listener(const std::string &key_prefix)    = 0;
  virtual bool remove_listener(const std::string &key_prefix) = 0;
};

#endif // ICONFIGBACKEND_HH
