// Copyright (C) 2014 Rob Caelers <robc@krandor.nl>
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

#include "Ui.hh"

#include "utils/Exception.hh"
#include "utils/AssetPath.hh"
#include "ui/UiTypes.hh"
#include "commonui/nls.h"

using namespace workrave;
using namespace workrave::utils;

const std::string
Ui::get_break_name(BreakId id)
{
  switch (id)
    {
    case BREAK_ID_MICRO_BREAK:
      return _("Micro-break");

    case BREAK_ID_REST_BREAK:
      return _("Rest break");

    case BREAK_ID_DAILY_LIMIT:
      return _("Daily limit");

    default:
      throw Exception("Invalid break id");
    }
}

const std::string
Ui::get_break_icon_filename(BreakId id)
{
  std::string filename;

  switch (id)
    {
    case BREAK_ID_MICRO_BREAK:
      filename = "timer-micro-break.png";
      break;

    case BREAK_ID_REST_BREAK:
      filename = "timer-rest-break.png";
      break;

    case BREAK_ID_DAILY_LIMIT:
      filename = "timer-daily.png";
      break;

    default:
      throw Exception("Invalid break id");
    }

  return AssetPath::complete_directory(filename, SearchPathId::Images);
}

const std::string
Ui::get_sound_event_name(SoundEvent event)
{
  switch (event)
    {
    case SoundEvent::BreakPrelude:
      return _("Break prompt");

    case SoundEvent::BreakIgnored:
      return _("Break ignored");

    case SoundEvent::RestBreakStarted:
      return _("Rest break started");

    case SoundEvent::RestBreakEnded:
      return _("Rest break ended");

    case SoundEvent::MicroBreakStarted:
      return _("Micro-break started");

    case SoundEvent::MicroBreakEnded:
      return _("Micro-break ended");

    case SoundEvent::DailyLimit:
      return _("Daily limit");

    case SoundEvent::ExerciseEnded:
      return _("Exercise ended");

    case SoundEvent::ExercisesEnded:
      return _("Exercises ended");

    case SoundEvent::ExerciseStep:
      return _("Exercise change");

    default:
      return _("?");
    }
}

const std::string
Ui::get_status_icon_filename(OperationModeIcon icon)
{
  std::string filename;

  switch (icon)
    {
    case OperationModeIcon::Normal:
      filename = "workrave-icon-medium.png";
      break;

    case OperationModeIcon::Quiet:
      filename = "workrave-quiet-icon-medium.png";
      break;

    case OperationModeIcon::Suspended:
      filename = "workrave-suspended-icon-medium.png";
      break;
    }

  return AssetPath::complete_directory(filename, SearchPathId::Images);
}
