// ISystemStateChangeMethod.hh -- interface for shutdown/suspend/hibernate
//
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
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
//

#ifndef ISYSTEMSTATECHANGEMETHOD_HH_
#define ISYSTEMSTATECHANGEMETHOD_HH_

class /*interface*/ ISystemStateChangeMethod
{

public:
  virtual ~ISystemStateChangeMethod() = default;
  ;
  virtual bool shutdown() { return false; }
  virtual bool suspend() { return false; }
  virtual bool hibernate() { return false; }
  virtual bool suspendHybrid() { return false; }

  virtual bool canShutdown() { return false; }
  virtual bool canSuspend() { return false; }
  virtual bool canHibernate() { return false; }
  virtual bool canSuspendHybrid() { return false; }

  virtual bool canDoAnything() { return canShutdown() || canSuspend() || canHibernate() || canSuspendHybrid(); }
};

#endif /* ISYSTEMSTATECHANGEMETHOD_HH_ */
