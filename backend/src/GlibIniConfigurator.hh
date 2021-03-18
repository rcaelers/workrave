// IniConfigurator.hh
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006, 2007 Rob Caelers <robc@krandor.nl>
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

#ifndef GLIBINICONFIGURATOR_HH
#define GLIBINICONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <glib.h>

#include "IConfigBackend.hh"

class GlibIniConfigurator : public virtual IConfigBackend
{
public:
  GlibIniConfigurator();
  ~GlibIniConfigurator() override;

  bool load(std::string filename) override;
  bool save(std::string filename) override;
  bool save() override;

  bool remove_key(const std::string &key) override;
  bool get_value(const std::string &key, VariantType type, Variant &value) const override;
  bool set_value(const std::string &key, Variant &value) override;

private:
  void split_key(const std::string &key, std::string &group, std::string &out_key) const;
  std::string key_inify(const std::string &key) const;

private:
  GKeyFile *config{nullptr};

  std::string last_filename;
};

#endif // GLIBINICONFIGURATOR_HH
