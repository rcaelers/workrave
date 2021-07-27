// Copyright (C) 2002, 2004, 2006, 2007, 2010 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef HARPOON_HELPER_H
#define HARPOON_HELPER_H

#include <windows.h>

#include "HarpoonHelper.h"
#include "harpoon.h"

#define HARPOON_HELPER_WINDOW_CLASS "HarpoonHelperNotificationWindow"

typedef enum
{
  HARPOON_HELPER_NOTHING = -1,
  HARPOON_HELPER_INIT = 0,
  HARPOON_HELPER_EXIT,
  HARPOON_HELPER_BLOCK,
  HARPOON_HELPER_UNBLOCK,
  HARPOON_HELPER_EVENT__SIZEOF

} HarpoonHelperEventType;

class HarpoonHelper
{
public:
  HarpoonHelper(char *args);
  virtual ~HarpoonHelper();

  bool init(HINSTANCE hInstance);
  void run();
  void terminate();

private:
  HINSTANCE hInstance;
  HWND notification_window;
  char *args;

  char critical_filename_list[HARPOON_MAX_UNBLOCKED_APPS][511];

  ATOM notification_class;

  static LRESULT CALLBACK harpoon_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  void init_critical_filename_list();
  bool check_for_taskmgr_debugger(char *out);
};

#endif // HARPOON_HELPER_H
