//
// Copyright (C) 2007, 2011, 2012 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include "debug.hh"
#include "utils/TimeSource.hh"

using namespace workrave::utils;

ITimeSource::Ptr TimeSource::source;
gint64 TimeSource::synced_real_time = 0;
gint64 TimeSource::synced_monotonic_time = 0;

gint64
TimeSource::get_real_time_usec()
{
  if (source != NULL)
    {
      return source->get_real_time_usec();
    }
  return g_get_real_time();
}

gint64
TimeSource::get_monotonic_time_usec()
{
  if (source != NULL)
    {
      return source->get_monotonic_time_usec();
    }
  return g_get_monotonic_time();
}

gint64
TimeSource::get_real_time_sec()
{
  return get_real_time_usec() / G_USEC_PER_SEC;
}

gint64
TimeSource::get_monotonic_time_sec()
{
  return get_monotonic_time_usec() / G_USEC_PER_SEC;
}

gint64
TimeSource::get_real_time_sync()
{
  return synced_real_time / G_USEC_PER_SEC;
}

gint64
TimeSource::get_monotonic_time_sync()
{
  return synced_monotonic_time / G_USEC_PER_SEC;
}

void
TimeSource::sync()
{
  synced_monotonic_time = get_monotonic_time_usec();
  synced_real_time = get_real_time_usec();
}

