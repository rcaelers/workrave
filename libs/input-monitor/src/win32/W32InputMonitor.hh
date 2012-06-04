// W32InputMonitor.hh --- ActivityMonitor for W32
//
// Copyright (C) 2002, 2004, 2006, 2007, 2012 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef W32INPUTMONITOR_HH
#define W32INPUTMONITOR_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <windows.h>
#include "InputMonitor.hh"
#include "config/IConfigurator.hh"

typedef union HarpoonEventUnion HarpoonEvent;

using namespace workrave::config;

//! Activity monitor for a local X server.
class W32InputMonitor :
  public InputMonitor
{
public:
  //! Constructor.
  W32InputMonitor(IConfigurator::Ptr config);

  //! Destructor.
  virtual ~W32InputMonitor();

  bool init();
  void terminate() ;

private:
  static W32InputMonitor *singleton;
  static void on_harpoon_event(HarpoonEvent *event);
  
  IConfigurator::Ptr config;
};

#endif // W32INPUTMONITOR_HH
