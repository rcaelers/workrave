// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#ifndef W32SHUTDOWN_HH_
#define W32SHUTDOWN_HH_

#include "session/ISystemStateChangeMethod.hh"

class W32Shutdown : public ISystemStateChangeMethod
{
public:
  W32Shutdown();
  virtual ~W32Shutdown(){};

  virtual bool shutdown();
  virtual bool canShutdown()
  {
    return shutdown_supported;
  }

private:
  bool shutdown_helper(bool for_real);
  bool shutdown_supported;
};

#endif /* W32SHUTDOWN_HH_ */
