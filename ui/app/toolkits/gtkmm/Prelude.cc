// Copyright (C) 2001 - 2017 Rob Caelers & Raymond Penners
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

#include "Prelude.hh"

#include "commonui/nls.h"

using namespace workrave;
using namespace workrave::utils;

std::string
Prelude::get_title(BreakId break_id) const
{
  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      return _("Time for a micro-break?");
    case BREAK_ID_REST_BREAK:
      return _("You need a rest break...");
    case BREAK_ID_DAILY_LIMIT:
      return _("You should stop for today...");
    default:
      return "";
    }
}

std::string
Prelude::get_progress_text(IApp::PreludeProgressText text)
{
  switch (text)
    {
    case IApp::PreludeProgressText::BreakIn:
      return _("Break in %s");
    case IApp::PreludeProgressText::DisappearsIn:
      return _("Disappears in %s");
    case IApp::PreludeProgressText::SilentIn:
      return _("Silent in %s");
    }
  return "";
}

