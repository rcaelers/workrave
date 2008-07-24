// NetworkHandler.hh --- Network Handler
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// $Id: NetworkHandler.hh 1301 2007-08-30 21:25:47Z rcaelers $
//

#ifndef NETWORKHANDLER_HH
#define NETWORKHANDLER_HH

#include <stdio.h>

#include "preinclude.h"
#include <string>

#include "INetwork.hh"
#include "ILinkEventListener.hh"
#include "LinkEvent.hh"

using namespace workrave;

class NetworkHandler :
  public ILinkEventListener
{
public:
  NetworkHandler();
  ~NetworkHandler();

  void init();
  
private:

  //
  virtual void event_received(LinkEvent *event);
  
private:
};

#endif // NETWORKHANDLER_HH
