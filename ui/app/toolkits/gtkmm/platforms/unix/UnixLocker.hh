// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#ifndef UNIXLOCKER_HH
#define UNIXLOCKER_HH

#include <vector>
#include <gtk/gtk.h>
#include <sigc++/sigc++.h>

#include "ui/Locker.hh"

namespace Gtk
{
  class Window;
}

class UnixLocker : public Locker
{
public:
  UnixLocker() = default;

  void set_window(GdkWindow *window);

  bool can_lock() override;
  void prepare_lock() override;
  void lock() override;
  void unlock() override;

private:
  bool lock_internal();
  bool on_lock_retry_timer();

  static GdkFilterReturn event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data);
  void handle_screen_lock_keystroke(unsigned int keycode, unsigned int modifier_state);
  void query_desktop_lock_shortcuts();
  void set_default_lock_shortcuts();
  void add_keybinding_shortcut(const char *binding, const char *schema_name, const char *key_name);
  bool parse_keybinding(const char *binding, unsigned long *keysym, unsigned int *modifiers);

#if !GTK_CHECK_VERSION(3, 24, 0)
  GdkDevice *keyboard{nullptr};
  GdkDevice *pointer{nullptr};
#endif
  GdkWindow *grab_window{nullptr};
  bool grab_wanted{false};
  bool grabbed{false};
  int xi_opcode{-1};
  sigc::connection grab_retry_connection;

  struct Shortcut
  {
    unsigned long keysym{0};
    unsigned int modifiers{0};
    bool valid{false};
  };
  std::vector<Shortcut> lock_shortcuts;
};

#endif // UNIXLOCKER_HH
