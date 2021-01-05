// SystemLockCommandline.hh -- support for locking the system using command line
//
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
// All rights reserved.
// Uses some code and ideas from the KShutdown utility: file src/actions/lock.cpp
// Copyright (C) 2009  Konrad Twardowski
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
//
#ifndef SYSTEMLOCKCOMMANDLINE_HH
#define SYSTEMLOCKCOMMANDLINE_HH

#include "IScreenLockMethod.hh"
#include "stdlib.h"
// A method of locking the screen that
// does that by executing a command
class ScreenLockCommandline : public IScreenLockMethod
{
public:
  // the parameter 'parameters' may be NULL, in which case it is assumed that the
  // program does not take any parameters
  // async - whether to invoke the program syncronously (async = false, wait for the command
  // to complete) or asynchronously (async = true)
  ScreenLockCommandline(const char *program_name, const char *parameters, bool async = false);
  ~ScreenLockCommandline()
  {
    if (cmd != NULL)
      {
        free(cmd);
        cmd = NULL;
      }
  }

  virtual bool is_lock_supported() { return cmd != NULL; }
  virtual bool lock();

private:
  bool invoke(const gchar *command, bool async);

  char *cmd;
  const bool async;
};

#endif
