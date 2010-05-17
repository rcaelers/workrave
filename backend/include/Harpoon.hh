// Harpoon.hh --- ActivityMonitor for W32
//
// Copyright (C) 2002, 2004, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
// Copyright (C) 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef HARPOON_HH
#define HARPOON_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <windows.h>
#include "harpoon.h"

typedef union HarpoonEventUnion HarpoonEvent;

//! Activity monitor for a local X server.
class Harpoon
{
public:
  //! Constructor.
  Harpoon();

  //! Destructor.
  virtual ~Harpoon();

  static bool init(HarpoonHookFunc func);
  static void terminate();
  static void block_input();
  static void unblock_input();

private:
  static void init_critical_filename_list();
  static bool check_for_taskmgr_debugger( char *out );
  static void on_harpoon_event(HarpoonEvent *event);

  static bool is_64bit_windows();
  static void start_harpoon_helper();
  static void stop_harpoon_helper();

  static HWND recursive_find_window(HWND hwnd, LPCTSTR lpClassName);
  
  static HWND helper_window;

  static char critical_filename_list[HARPOON_MAX_UNBLOCKED_APPS][511];
};
#endif // HARPOON_HH
