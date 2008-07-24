// Trackable.cc
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

static const char rcsid[] = "$Id: Locale.cc 1356 2007-10-22 18:22:13Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include "nls.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>
#include <string>
#include <list>
#include <algorithm>
#include <typeinfo>

#include "Trackable.hh"
#include "StringUtil.hh"

Trackable::Trackables Trackable::trackables;

Trackable::Trackable()
{
  void *array[100];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 100);
  strings = backtrace_symbols(array, size);

  for (i = 0; i < size; i++)
    {
      trace += strings[i];
      trace += "\n";
    }

  trackables.push_back(this);
}


Trackable::~Trackable()
{
  TrackableIter i = find(trackables.begin(), trackables.end(), this);
  
  assert(i != trackables.end());
  
  trackables.remove(this);
}


void
Trackable::dump()
{
  cout << "Left over:" << endl;

  for (TrackableIter i = trackables.begin(); i != trackables.end(); i++)
    {
      cout << typeid(*(*i)).name() << endl;
      cout << (*i)->trace << endl; 
    }
}

