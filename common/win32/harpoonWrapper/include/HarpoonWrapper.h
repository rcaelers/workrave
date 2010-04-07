// Harpoon.hh --- ActivityMonitor for W32
//
// Copyright (C) 2002, 2004, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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
#include "HarpoonWrapper.h"
#include "harpoon.h"

typedef enum
{
  HARPOON_WRAPPER_NOTHING = -1,
  HARPOON_WRAPPER_BLOCK = 0,
  HARPOON_WRAPPER_UNBLOCK,
  HARPOON_WRAPPER_EVENT__SIZEOF

} HarpoonWrapperEventType;


//! Activity monitor for a local X server.
class Harpoon
{
public:
  //! Constructor.
  Harpoon();

  //! Destructor.
  virtual ~Harpoon();

  bool init(HINSTANCE hInstance);
  void run();
  void terminate();

private:
  HINSTANCE hInstance;
  HWND notification_window;
  char critical_filename_list[ HARPOON_MAX_UNBLOCKED_APPS ][ 511 ];

  ATOM notification_class;
  static LRESULT CALLBACK harpoon_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


  void init_critical_filename_list();
  bool check_for_taskmgr_debugger( char *out );
  //static void on_harpoon_event(HarpoonEvent *event);
};

#endif // HARPOON_HH
