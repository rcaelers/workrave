// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#ifndef DEBUGDIALOG_HH
#define DEBUGDIALOG_HH

#include <string>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textbuffer.h>

#include "utils/Diagnostics.hh"

namespace Gtk
{
  class TextView;
}

class DebugDialog
  : public Gtk::Dialog
  , public DiagnosticsSink
{
public:
  DebugDialog();
  ~DebugDialog() override;

  int run();

  void diagnostics_log(const std::string &log) override;

private:
  void init();
  void on_response(int response) override;

  Gtk::TextView *text_view{nullptr};
  Gtk::ScrolledWindow scrolled_window;
  Glib::RefPtr<Gtk::TextBuffer> text_buffer;
};

#endif // DEBUGWINDOW_HH
