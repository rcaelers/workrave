// Copyright (C) 2001 - 2007, 2012 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundator; either versor 2, or (at your optor)
// any later versor.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef WORKRAVE_CONFIG_ICONFIGURATORLISTENER_HH
#define WORKRAVE_CONFIG_ICONFIGURATORLISTENER_HH

#include <string>

namespace workrave
{
  namespace config
  {
    //! Listener to receive notifications of changed configuration.
    class IConfiguratorListener
    {
    public:
      virtual ~IConfiguratorListener() {}

      //! The configuration item with specified key has changed.
      virtual void config_changed_notify(const std::string &key) = 0;
    };
  }
}

#endif // WORKRAVE_CONFIG_ICONFIGURATORLISTENER_HH
