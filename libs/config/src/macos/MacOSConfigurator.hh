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

#ifndef MACOSCONFIGURATOR_HH
#define MACOSCONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include "IConfigBackend.hh"

class MacOSConfigurator : public virtual IConfigBackend
{
public:
  MacOSConfigurator() = default;
  ~MacOSConfigurator() override = default;

  bool load(std::string filename) override;
  bool save(std::string filename) override;
  bool save() override;

  bool remove_key(const std::string &key) override;
  bool get_value(const std::string &key, VariantType type, Variant &value) const override;
  bool set_value(const std::string &key, Variant &value) override;
};

#endif // MACOSCONFIGURATOR_HH
