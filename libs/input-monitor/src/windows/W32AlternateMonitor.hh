// Copyright (C) 2007, 2012 Ray Satiro <raysatiro@yahoo.com>
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
// See comments in W32AlternateMonitor.cc

#ifndef W32ALTERNATEMONITOR_HH
#define W32ALTERNATEMONITOR_HH

#include <assert.h>
#include <windows.h>

#include "InputMonitor.hh"
#include "config/IConfigurator.hh"

using namespace workrave::config;

class W32AlternateMonitor : public InputMonitor
{
public:
  W32AlternateMonitor(workrave::config::IConfigurator::Ptr config);
  virtual ~W32AlternateMonitor();
  bool init();
  void terminate();

protected:
  static DWORD WINAPI thread_Monitor(LPVOID);

private:
  void Monitor();
  void Update(LASTINPUTINFO *);

  workrave::config::IConfigurator::Ptr config;
  bool initialized;
  int interval;
  HANDLE thread_abort_event;
  HANDLE thread_handle;
  volatile DWORD thread_id;
};

#endif // W32ALTERNATEMONITOR_HH
