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

#include "DebugDialog.hh"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gtkmm/textview.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/stock.h>

#include "commonui/nls.h"
#include "debug.hh"

using namespace workrave;

DebugDialog::DebugDialog()
  : Gtk::Dialog(_("Debug log"), false)
{
  TRACE_ENTRY();
  set_default_size(1024, 800);

  text_buffer = Gtk::TextBuffer::create();
  text_view = Gtk::manage(new Gtk::TextView(text_buffer));
  text_view->set_cursor_visible(false);
  text_view->set_editable(false);

  scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled_window.add(*text_view);

  Gtk::HBox *box = Gtk::manage(new Gtk::HBox(false, 6));
  box->pack_start(scrolled_window, true, true, 0);

  get_vbox()->pack_start(*box, true, true, 0);

  add_button(_("Close"), Gtk::RESPONSE_CLOSE);

  show_all();
}

DebugDialog::~DebugDialog()
{
  TRACE_ENTRY();
  Diagnostics::instance().disable();
}

void
DebugDialog::diagnostics_log(const std::string &log)
{
  Gtk::TextIter iter = text_buffer->end();
  iter = text_buffer->insert(iter, log + "\n");
  Glib::RefPtr<Gtk::Adjustment> a = scrolled_window.get_vadjustment();
  a->set_value(a->get_upper());
}

void
DebugDialog::init()
{
  Diagnostics::instance().enable(this);
}

int
DebugDialog::run()
{
  TRACE_ENTRY();
  init();
  show_all();
  return 0;
}

void
DebugDialog::on_response(int response)
{
  (void)response;
  TRACE_ENTRY();
  Diagnostics::instance().disable();
}
