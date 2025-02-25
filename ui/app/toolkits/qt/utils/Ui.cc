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

using namespace workrave::utils;

auto
Ui::get_break_name(workrave::BreakId id) -> QString
{
  switch (id)
    {
    case workrave::BREAK_ID_MICRO_BREAK:
      return tr("Micro-break");

    case workrave::BREAK_ID_REST_BREAK:
      return tr("Rest break");

    case workrave::BREAK_ID_DAILY_LIMIT:
      return tr("Daily limit");

    default:
      throw Exception("Invalid break id");
    }
}

auto
Ui::get_break_icon_filename(workrave::BreakId id) -> QString
{
  std::string filename;

  switch (id)
    {
    case workrave::BREAK_ID_MICRO_BREAK:
      filename = "timer-micro-break.png";
      break;

    case workrave::BREAK_ID_REST_BREAK:
      filename = "timer-rest-break.png";
      break;

    case workrave::BREAK_ID_DAILY_LIMIT:
      filename = "timer-daily.png";
      break;

    default:
      throw Exception("Invalid break id");
    }

  return QString::fromStdString(AssetPath::complete_directory(filename, SearchPathId::Images));
}

auto
Ui::get_sound_event_name(SoundEvent event) -> QString
{
  switch (event)
    {
    case SoundEvent::BreakPrelude:
      return tr("Break prompt");

    case SoundEvent::BreakIgnored:
      return tr("Break ignored");

    case SoundEvent::RestBreakStarted:
      return tr("Rest break started");

    case SoundEvent::RestBreakEnded:
      return tr("Rest break ended");

    case SoundEvent::MicroBreakStarted:
      return tr("Micro-break started");

    case SoundEvent::MicroBreakEnded:
      return tr("Micro-break ended");

    case SoundEvent::DailyLimit:
      return tr("Daily limit");

    case SoundEvent::ExerciseEnded:
      return tr("Exercise ended");

    case SoundEvent::ExercisesEnded:
      return tr("Exercises ended");

    case SoundEvent::ExerciseStep:
      return tr("Exercise change");

    default:
      return tr("?");
    }
}

auto
Ui::get_status_icon_filename(OperationModeIcon icon) -> QString
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

  return QString::fromStdString(AssetPath::complete_directory(filename, SearchPathId::Images));
}
