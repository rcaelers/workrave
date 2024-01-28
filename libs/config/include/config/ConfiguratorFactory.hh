// Copyright (C) 2007, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_CONFIG_CONFIGURATORFACTORY_HH
#define WORKRAVE_CONFIG_CONFIGURATORFACTORY_HH

#include "IConfigurator.hh"

namespace workrave::config
{
  enum class ConfigFileFormat
  {
    Ini,
    Xml,
    Native
  };

  class ConfiguratorFactory
  {
  public:
    static IConfigurator::Ptr create(ConfigFileFormat fmt);
  };
} // namespace workrave::config

#endif // WORKRAVE_CONFIG_CONFIGURATORFACTORY_HH
