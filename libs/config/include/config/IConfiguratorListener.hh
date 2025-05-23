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

#ifndef WORKRAVE_CONFIG_ICONFIGURATORLISTENER_HH
#define WORKRAVE_CONFIG_ICONFIGURATORLISTENER_HH

#include <string>

namespace workrave::config
{
  //! Listener to receive notifications of changed configuration.
  class IConfiguratorListener
  {
  public:
    virtual ~IConfiguratorListener() = default;

    //! The configuration item with specified key has changed.
    virtual void config_changed_notify(const std::string &key) = 0;
  };
} // namespace workrave::config

#endif // WORKRAVE_CONFIG_ICONFIGURATORLISTENER_HH
