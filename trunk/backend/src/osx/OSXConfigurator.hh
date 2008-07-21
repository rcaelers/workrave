// OSXConfigurator.hh
//
// Copyright (C) 2008 Rob Caelers <robc@krandor.nl>
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
// $Id: OSXConfigurator.hh 1351 2007-10-14 20:56:54Z rcaelers $
//

#ifndef OSXCONFIGURATOR_HH
#define OSXCONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include "IConfigBackend.hh"

class OSXConfigurator :
  public virtual IConfigBackend
{
public:
  OSXConfigurator();
  virtual ~OSXConfigurator();

  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key);
  virtual bool get_value(const std::string &key, VariantType type, Variant &value) const;
  virtual bool set_value(const std::string &key, Variant &value);
};

#endif // OSXCONFIGURATOR_HH
