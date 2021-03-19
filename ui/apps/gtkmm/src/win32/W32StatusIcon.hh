// W32StatusIcon.hh --- Window Notification Icon
//
// Copyright (C) 2010, 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef W32STATUSICON_HH
#define W32STATUSICON_HH

#include "config.h"

#include <string>

#include <glibmm.h>
#include <gdkmm.h>

#include <windows.h>
#include <commctrl.h>
#undef __out
#undef __in

class W32StatusIcon
{
public:
  W32StatusIcon();
  virtual ~W32StatusIcon();

  void set(const Glib::RefPtr<Gdk::Pixbuf> &pixbuf);
  void set_tooltip(const Glib::ustring &text);
  void show_balloon(std::string id, const Glib::ustring &balloon);
  void set_visible(bool visible = true);
  bool get_visible() const;
  bool is_embedded() const;

  sigc::signal<void> signal_activate();
  sigc::signal<void, std::string> signal_balloon_activate();
  sigc::signal<void, guint, guint32> signal_popup_menu();

private:
  std::string current_id;
  bool visible;

  NOTIFYICONDATAW nid;

  static HWND tray_hwnd;
  static UINT wm_taskbarcreated;

  void init();
  void cleanup();
  void add_tray_icon();

  static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  sigc::signal<void> activate_signal;
  sigc::signal<void, std::string> balloon_activate_signal;
  sigc::signal<void, guint, guint32> popup_menu_signal;
};

#endif // W32STATUSICON_HH
