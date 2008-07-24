// Trackable.hh
//
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
// $Id: Trackable.hh 1356 2007-10-22 18:22:13Z rcaelers $
//

#ifndef TRACKABLE_HH
#define TRACKABLE_HH

#include <map>
#include <string>

#include <glib.h>

using namespace std;

class Trackable
{
public:
  Trackable();
  virtual ~Trackable();

  static void dump();
  
private:
  void init();
  virtual void virt() {}
  
  typedef std::list<Trackable *> Trackables;
  typedef Trackables::iterator TrackableIter;
  typedef Trackables::const_iterator TrackableCIter;

  std::string trace;

  static Trackables trackables;
};

#endif // TRACKABLE_HH
