// CollectiveJoinDialog.cc --- CollectiveJoin dialog
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <assert.h>

#include "CollectiveJoinDialog.hh"

#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"
#include "Configurator.hh"
#include "GUIControl.hh"
#include "Util.hh"

using std::cout;
using SigC::slot;


CollectiveJoinDialog::CollectiveJoinDialog()
  : Gtk::Dialog("Network connect", true, false)
{
  TRACE_ENTER("CollectiveJoinDialog::CollectiveJoinDialog");

  string text =
    _("Enter the host name and port number of a computer\n"
      "in the network you wish to connect to.");
  
  // Title
  Gtk::HBox *title_box = manage(new Gtk::HBox(false));
  string title_icon = Util::complete_directory("collective.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *title_img = manage(new Gtk::Image(title_icon));
  Gtk::Label *title_lab = manage(new Gtk::Label(text));
  title_box->pack_start(*title_img, false, false, 6);
  title_box->pack_start(*title_lab, false, true, 6);

  // Entry
  host_entry = manage(new Gtk::Entry());
  port_entry = manage(new Gtk::SpinButton());
  Gtk::Label *host_label = manage(new Gtk::Label(_("Host name")));
  Gtk::Label *port_label = manage(new Gtk::Label(_("Port")));

  port_entry->set_range(1024, 65535);
  port_entry->set_increments(1, 10);
  port_entry->set_numeric(true);
  port_entry->set_width_chars(10);
  
  host_entry->set_width_chars(40);

  Gtk::Alignment *port_al = manage(new Gtk::Alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_BOTTOM, 0.0, 0.0));
  port_al->add(*port_entry);

  Gtk::Table *entry_table = manage(new Gtk::Table(2, 2, false));
  entry_table->set_row_spacings(2);
  entry_table->set_col_spacings(6);
  int y = 0;
  entry_table->attach(*host_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  entry_table->attach(*host_entry, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  y++;
  entry_table->attach(*port_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  entry_table->attach(*port_al, 1, 2, y, y+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);

  //
  startup_button = manage(new Gtk::CheckButton(_("Connect at start-up")));
  y++;
  entry_table->attach(*startup_button, 1, 2, y, y+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
  
  // Page
  Gtk::VBox *page = new Gtk::VBox(false, 6);
  page->pack_start(*title_box, false, true, 0);
  page->pack_start(*entry_table, false, true, 0);
  page->set_border_width(6);

  Gtk::Frame *frame = manage(new Gtk::Frame(_("Connection details")));
  frame->set_border_width(6);
  frame->add(*page);

  get_vbox()->pack_start(*frame, false, false, 0);

  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

  show_all();

  TRACE_EXIT();
}


//! Destructor.
CollectiveJoinDialog::~CollectiveJoinDialog()
{
  TRACE_ENTER("CollectiveJoinDialog::~CollectiveJoinDialog");
  TRACE_EXIT();
}

void
CollectiveJoinDialog::init()
{
  int value;
  Configurator *c = GUIControl::get_instance()->get_configurator();
  bool is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PORT, &value);
  if (!is_set)
    {
      value = DEFAULT_PORT;
    }
  
  port_entry->set_value(value);
}

int
CollectiveJoinDialog::run()
{
  init();
  
  int id = Gtk::Dialog::run();

  if (id == Gtk::RESPONSE_OK)
    {
      DistributionManager *dist_manager = DistributionManager::get_instance();

      string peer = "tcp://" + host_entry->get_text() + ":" + port_entry->get_text();
      if (startup_button->get_active())
        {
          dist_manager->add_peer(peer);
        }
      else
        {
          dist_manager->join(peer);
        }
    }
  GUIControl::get_instance()->get_configurator()->save();
}
