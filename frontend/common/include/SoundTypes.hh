// SoundType.hh
//
// Copyright (C) 2002 - 2010 Rob Caelers & Raymond Penners
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

#ifndef SOUNDTYPES_HH
#define SOUNDTYPES_HH

enum SoundCapability
{
  SOUND_CAP_EVENTS = 0,
  SOUND_CAP_EDIT,
  SOUND_CAP_VOLUME,
  SOUND_CAP_MUTE,
  SOUND_CAP_EOS_EVENT,
};

enum SoundEvent
{
  SOUND_MIN           = 0,
  SOUND_BREAK_PRELUDE = 0,
  SOUND_BREAK_IGNORED,
  SOUND_REST_BREAK_STARTED,
  SOUND_REST_BREAK_ENDED,
  SOUND_MICRO_BREAK_STARTED,
  SOUND_MICRO_BREAK_ENDED,
  SOUND_DAILY_LIMIT,
  SOUND_EXERCISE_ENDED,
  SOUND_EXERCISES_ENDED,
  SOUND_EXERCISE_STEP,
  SOUND_MAX
};

#endif // SOUNDTYPES_HH
