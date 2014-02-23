// SoundPlayer.hh
//
// Copyright (C) 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011, 2013 Rob Caelers & Raymond Penners
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

#ifndef ISOUNDPLAYER_HH
#define ISOUNDPLAYER_HH

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace workrave
{
  namespace audio
  {
    enum SoundEvent
      {
        SOUND_MIN = 0,
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
    
    enum SoundCapability
      {
        SOUND_CAP_EVENTS = 0,
        SOUND_CAP_EDIT,
        SOUND_CAP_VOLUME,
        SOUND_CAP_MUTE,
        SOUND_CAP_EOS_EVENT
      };
    
    class ISoundPlayer
    {
    public:
      typedef boost::shared_ptr<ISoundPlayer> Ptr;
      
      static Ptr create();
      
      virtual ~ISoundPlayer() {}
      
      virtual void play_sound(SoundEvent snd, const std::string &wavfile, bool mute_after_playback, int volume) = 0;
      virtual void play_sound(const std::string &wavfile, int volume) = 0;
      
      virtual void init() = 0;
      virtual bool capability(SoundCapability cap) = 0;
      virtual void restore_mute() = 0;
    };
  }
}

#endif // ISOUNDPLAYER_HH
