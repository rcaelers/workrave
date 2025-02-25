// MicroBreakWindow.hh --- window for the microbreak
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers <robc@krandor.nl>
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

#ifndef MICROBREAKWINDOW_HH
#define MICROBREAKWINDOW_HH

#include "BreakWindow.hh"
#include "ui/GUIConfig.hh"

namespace workrave
{
  class ITimer;
}

namespace Gtk
{
  class Label;
}

class TimeBar;
class Frame;

class MicroBreakWindow : public BreakWindow
{
public:
  MicroBreakWindow(std::shared_ptr<IApplicationContext> app, HeadInfo head, BreakFlags break_flags, BlockMode mode);
  ~MicroBreakWindow() override = default;

  void set_progress(int value, int max_value) override;
  void heartbeat();

protected:
  Gtk::Widget *create_gui() override;
  void on_restbreaknow_button_clicked();

private:
  void update_break_window() override;
  void update_time_bar();
  void update_label();
  Gtk::Button *create_restbreaknow_button(bool label);

private:
  TimeBar *time_bar{nullptr};
  Gtk::Label *label{nullptr};
  int progress_value{0};
  int progress_max_value{0};
  bool is_flashing{false};
  bool fixed_size{false};
};

#endif // MICROBREAKWINDOW_HH
