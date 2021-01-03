// Copyright (C) 2002 - 2014 Rob Caelers & Raymond Penners
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

#ifndef WORKRAVE_AUDIO_ISOUNDPLAYER_HH
#define WORKRAVE_AUDIO_ISOUNDPLAYER_HH

#include <string>

#include <memory>

namespace workrave
{
  namespace audio
  {

    enum class SoundCapability
      {
        VOLUME,
        MUTE,
        EOS_EVENT
      };

    class ISoundPlayer
    {
    public:
      using Ptr = std::shared_ptr<ISoundPlayer>;

      virtual ~ISoundPlayer() = default;

      virtual void init() = 0;
      virtual bool capability(SoundCapability cap) = 0;
      virtual void restore_mute() = 0;
      virtual void play_sound(const std::string &wavfile, bool mute_after_playback, int volume) = 0;
    };

    class SoundPlayerFactory
    {
    public:
      static ISoundPlayer::Ptr create();
    };
  }
}

#endif // WORKRAVE_AUDIO_ISOUNDPLAYER_HH
