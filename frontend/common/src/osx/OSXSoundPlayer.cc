// OSXSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2007 Raymond Penners & Ray Satiro
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

static const char rcsid[] = "$Id: OSXSoundPlayer.cc 1275 2007-08-16 21:30:44Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <strings.h>

#include "OSXSoundPlayer.hh"
#include "SoundPlayer.hh"
#include "Util.hh"

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

static struct SoundRegistry
{
  const char *event_label;
  const char *wav_file;
  const char *friendly_name;
} sound_registry[] =
{
  { "WorkraveBreakPrelude", "break-prelude.wav",
    "Break prompt" },
  { "WorkraveBreakIgnored", "break-ignored.wav",
    "Break ignored" },
  { "WorkraveRestBreakStarted", "rest-break-started.wav",
    "Rest break started" },
  { "WorkraveRestBreakEnded", "rest-break-ended.wav",
    "Rest break ended" },
  { "WorkraveMicroBreakStarted", "micro-break-started.wav",
    "Micro-break started" },
  { "WorkraveMicroBreakEnded", "micro-break-ended.wav",
    "Micro-break ended" },
  { "WorkraveDailyLimit", "daily-limit.wav",
    "Daily limit" },
  { "WorkraveExerciseEnded", "exercise-ended.wav",
    "Exercise ended" },
  { "WorkraveExercisesEnded", "exercises-ended.wav",
    "Exercises ended" },
};


OSXSoundPlayer::OSXSoundPlayer()
{
  EnterMovies();
}


OSXSoundPlayer::~OSXSoundPlayer()
{
}

void
OSXSoundPlayer::play_sound(Sound snd)
{
  TRACE_ENTER_MSG( "OSXSoundPlayer::play_sound", sound_registry[snd].friendly_name );

  string file = Util::complete_directory(sound_registry[snd].wav_file, Util::SEARCH_PATH_SOUNDS);

  OSErr err;
  FSSpec spec;
  const char *fname = file.c_str();
  Movie movie;

  FSRef fref;
  err = FSPathMakeRef((const UInt8 *) fname, &fref, NULL);
  if (err != noErr)
    {
      printf("FSPathMakeRef failed %d\n", err);
      return;
    }
  err = FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &spec, NULL);
  if (err != noErr)
    {
      printf("FSGetCatalogInfo failed %d\n", err);
      return;
    }

  short movieResFile = 0;
  err = OpenMovieFile(&spec, &movieResFile, fsRdPerm);
  if (err != noErr)
    {
      printf("OpenMovieFile failed %d\n", err);
      return;
    }

  Str255 name;
  err = NewMovieFromFile(&movie, movieResFile, NULL, name, 0, NULL);
  if (err != noErr)
    {
      printf("NewMovieFromFile failed %d\n", err);
      err = CloseMovieFile(movieResFile);
      if (err != noErr)
        {
          printf("CloseMovieFile failed %d\n", err);
        }
      return;
    }


  // play the movie once thru
  StartMovie(movie);

  while (!IsMovieDone(movie))
    {
      MoviesTask(movie, 0);
    }

  DisposeMovie(movie);

  TRACE_EXIT();
}
