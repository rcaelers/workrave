// Test.hh --- Whitebox testing code
//
// Copyright (C) 2006, 2007, 2008 Rob Caelers
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

#ifndef TEST_H
#define TEST_H

#ifdef HAVE_DISTRIBUTION
#include "LinkedHistoryManager.hh"
#endif

class Test
{
public:
  static Test *get_instance();

  void quit();
  void fake_monitor(int state);
  int get_remote_state();
  void tick(int ticks);
  void set_time(time_t time);
  void init_time(time_t time);

#if defined(HAVE_DISTRIBUTION) && defined(HAVE_TESTS)
  void get_activity_history(LinkedHistoryManager::Activity **activity);
  void get_settings_history(LinkedHistoryManager::Settings **settings);
#endif

private:
  //! The one and only instance
  static Test *instance;
};



//! Returns the singleton Test instance.
inline Test *
Test::get_instance()
{
  if (instance == NULL)
    {
      instance = new Test();
    }

  return instance;
}

#endif // TEST_H
