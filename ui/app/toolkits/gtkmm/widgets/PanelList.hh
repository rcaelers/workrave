// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#ifndef PANEL_LIST_HH
#define PANEL_LIST_HH

#include <string>

#include <gtkmm.h>

class PanelList : public Gtk::ScrolledWindow
{
public:
  using activated_signal_t = sigc::signal<void(const std::string &id)>;

  PanelList();
  ~PanelList() override = default;

  void add_row(const std::string &id, const std::string &label, const std::string &image);
  activated_signal_t &signal_activated();

private:
  Gtk::ListBox *list_box{nullptr};
  activated_signal_t activated_signal;
};

#endif // PANEL_LIST_HH
