// NetworkJoinDialog.cc --- NetworkJoin dialog
//
// Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
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

using namespace std;

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <assert.h>

#include "NetworkJoinDialog.hh"

#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"
#include "Configurator.hh"
#include "GUIControl.hh"
#include "Util.hh"
#include "GtkUtil.hh"

NetworkJoinDialog::NetworkJoinDialog()
  : HigDialog(_("Network connect"), true, false)
{
  TRACE_ENTER("NetworkJoinDialog::NetworkJoinDialog");

  // Icon
  string title_icon = Util::complete_directory
    ("network.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *title_img = manage(new Gtk::Image(title_icon));
  Gtk::Alignment *img_aln
    = Gtk::manage(new Gtk::Alignment
                  (Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP, 0.0, 0.0));
  img_aln->add(*title_img);

  Gtk::Label *title_lab = manage(new Gtk::Label());
  Glib::ustring text = HigUtil::create_alert_text
    (_("Network connect"),
     _("Enter the host name and port number of a computer\n"
       "in the network you wish to connect to."));
  title_lab->set_markup(text);
  
  host_entry = manage(new Gtk::Entry());
  host_entry->set_width_chars(40);

  port_entry = manage(new Gtk::SpinButton());
  port_entry->set_range(1024, 65535);
  port_entry->set_increments(1, 10);
  port_entry->set_numeric(true);
  port_entry->set_width_chars(10);

  Gtk::Label *host_lab = manage(new Gtk::Label(_("Host name:")));
  Gtk::Label *port_lab = manage(new Gtk::Label(_("Port:")));

  startup_button = manage(new Gtk::CheckButton(_("Connect at start-up")));

  // Table
  Gtk::Table *table = manage(new Gtk::Table(4, 2, false));
  table->set_spacings(6);
  title_lab->set_alignment(0.0);
  table->attach(*title_lab, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 0, 6);
  GtkUtil::table_attach_left_aligned(*table, *host_lab, 0, 1);
  GtkUtil::table_attach_left_aligned(*table, *host_entry, 1, 1); 
  GtkUtil::table_attach_left_aligned(*table, *port_lab, 0, 2);
  GtkUtil::table_attach_left_aligned(*table, *port_entry, 1, 2);

  // Page
  Gtk::HBox *page = manage(new Gtk::HBox(false, 12));
  page->pack_start(*img_aln, false, true, 0);
  page->pack_start(*table, false, true, 0);

  get_vbox()->pack_start(*page, false, false, 0);

  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

  show_all();

  TRACE_EXIT();
}


//! Destructor.
NetworkJoinDialog::~NetworkJoinDialog()
{
  TRACE_ENTER("NetworkJoinDialog::~NetworkJoinDialog");
  TRACE_EXIT();
}

void
NetworkJoinDialog::init()
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
NetworkJoinDialog::run()
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
          dist_manager->connect(peer);
        }
    }
  GUIControl::get_instance()->get_configurator()->save();
  return id;
}
