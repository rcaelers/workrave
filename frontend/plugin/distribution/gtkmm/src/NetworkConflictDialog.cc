// NetworkConflictDialog.cc --- NetworkConflict dialog
//
// Copyright (C) 2002, 2003, 2006, 2007, 2009 Rob Caelers & Raymond Penners
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
// $Id: NetworkConflictDialog.cc 1301 2007-08-30 21:25:47Z rcaelers $

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#ifdef HAVE_UNISTD
#include <unistd.h>
#endif

#include <gtkmm/textview.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/stock.h>

#include "NetworkConflictDialog.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "Util.hh"
#include "GtkUtil.hh"

using namespace workrave;

NetworkConflictDialog::NetworkConflictDialog()
  : HigDialog(_("Configuration conflict"), true, false)
{
  TRACE_ENTER("NetworkConflictDialog::NetworkConflictDialog");

  set_default_size(600, 400);

  create_ui();

//   Gtk::HBox *box = manage(new Gtk::HBox(false, 6));
//   box->pack_start(*ui, true, true, 0);

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  show_all();

  TRACE_EXIT();
}


//! Destructor.
NetworkConflictDialog::~NetworkConflictDialog()
{
  TRACE_ENTER("NetworkConflictDialog::~NetworkConflictDialog");
  TRACE_EXIT();
}


void
NetworkConflictDialog::init()
{
//   ICore *core = CoreFactory::get_core();
}


void
NetworkConflictDialog::create_ui()
{
  // Info text
  const char *label =
    _("A connection has been established with a remote Workrave.\n"
      "This Workrave uses different break setting. Please resolve..");
  Gtk::Label *info_lab = Gtk::manage(new Gtk::Label(label));
  info_lab->set_alignment(0.0);
  get_vbox()->pack_start(*info_lab, false, false, 0);

  add_button("Use local settings", LOCAL);
  add_button("Use remote settings", REMOTE);
}


int
NetworkConflictDialog::run()
{
  TRACE_ENTER("NetworkConflictDialog::run");

  init();
  show_all();
  
  TRACE_EXIT();
  return 0;
}


void
NetworkConflictDialog::on_response(int response)
{
  (void) response;
  TRACE_ENTER("NetworkConflictDialog::on_response")
  TRACE_EXIT();
}

