// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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
#  include "config.h"
#endif

#include "Diagnostics.hh"

#include <ctime>

bool TracedFieldBase::debug = false;

void
Diagnostics::enable(DiagnosticsSink *sink)
{
  this->sink = sink;
  enabled = true;
  TracedFieldBase::debug = true;
  for (const auto &kv: topics)
    {
      kv.second();
    }
}

void
Diagnostics::disable()
{
  enabled = false;
  sink = nullptr;
  TracedFieldBase::debug = false;
}

void
Diagnostics::register_topic(const std::string &name, request_t func)
{
  topics[name] = func;
}

void
Diagnostics::unregister_topic(const std::string &name)
{
  topics.erase(name);
}

std::string
Diagnostics::trace_get_time()
{
  char logtime[128];
  time_t ltime = 0;

  time(&ltime);
  struct tm *tmlt = localtime(&ltime);
  strftime(logtime, 128, "%d %b %Y %H:%M:%S ", tmlt);
  return logtime;
}
