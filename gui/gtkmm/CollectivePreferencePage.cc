// CollectivePreferencePage.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#include "debug.hh"

#include <unistd.h>
#include "GUIControl.hh"
#include "CollectivePreferencePage.hh"
#include "Configurator.hh"
#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"

using std::cout;
using SigC::slot;


CollectivePreferencePage::CollectivePreferencePage()
  : Gtk::VBox(false, 6)
{
  TRACE_ENTER("CollectivePreferencePage::CollectivePreferencePage");

  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);  

  create_general_page(tnotebook);
  create_advanced_page(tnotebook);
  create_peers_page(tnotebook);

  init_page_values();
  
  tnotebook->show_all();

  pack_start(*tnotebook, false, false, 0);

  TRACE_EXIT();
}


CollectivePreferencePage::~CollectivePreferencePage()
{
  TRACE_ENTER("CollectivePreferencePage::~CollectivePreferencePage");
  TRACE_EXIT();
}


void
CollectivePreferencePage::create_general_page(Gtk::Notebook *tnotebook)
{
  Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = manage(new Gtk::Label("General"));
  // Gtk::Image *img = manage(new Gtk::Image();
  // box->pack_start(*img, false, false, 0);
  box->pack_start(*lab, false, false, 0);

  Gtk::VBox *gp = manage(new Gtk::VBox(false, 6));
  gp->set_border_width(6);

  // Main switch
  enabled_cb = manage(new Gtk::CheckButton("Collective enabled"));

  // Identity
  Gtk::Frame *id_frame = new Gtk::Frame("Identity");
  id_frame->set_border_width(6);

  username_entry = manage(new Gtk::Entry());
  password1_entry = manage(new Gtk::Entry());
  password2_entry = manage(new Gtk::Entry());
  
  Gtk::Label *username_label = manage(new Gtk::Label("Username"));
  Gtk::Label *password1_label = manage(new Gtk::Label("Pasword"));
  Gtk::Label *password2_label = manage(new Gtk::Label("Verify"));

  password1_entry->set_invisible_char('*');
  password2_entry->set_invisible_char('*'); 
  
  Gtk::Table *id_table = manage(new Gtk::Table(3, 2, false));
  id_table->set_row_spacings(2);
  id_table->set_col_spacings(6);
  id_table->set_border_width(6);
  int y = 0;
  id_table->attach(*username_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  id_table->attach(*username_entry, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  y++;
  id_table->attach(*password1_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  id_table->attach(*password1_entry, 1, 2, y, y+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
  y++;
  id_table->attach(*password2_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  id_table->attach(*password2_entry, 1, 2, y, y+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);

  id_frame->add(*id_table);
  
  gp->pack_start(*enabled_cb, false, false, 0);
  gp->pack_start(*id_frame, false, false, 0);

  box->show_all();
  tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*gp, *box));

  enabled_cb->signal_toggled().connect(SigC::slot(*this, &CollectivePreferencePage::on_enabled_toggled));
  username_entry->signal_changed().connect(SigC::slot(*this, &CollectivePreferencePage::on_username_changed));
  password1_entry->signal_changed().connect(SigC::slot(*this, &CollectivePreferencePage::on_password_changed));
  password2_entry->signal_changed().connect(SigC::slot(*this, &CollectivePreferencePage::on_password_changed));
}


void
CollectivePreferencePage::create_advanced_page(Gtk::Notebook *tnotebook)
{
  Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = manage(new Gtk::Label("Advanced"));
  // Gtk::Image *img = manage(new Gtk::Image();
  // box->pack_start(*img, false, false, 0);
  box->pack_start(*lab, false, false, 0);

  Gtk::VBox *gp = manage(new Gtk::VBox(false, 6));
  gp->set_border_width(6);

  Gtk::Frame *advanced_frame = new Gtk::Frame("Server settings");
  advanced_frame->set_border_width(6);

  port_entry = manage(new Gtk::SpinButton());
  attempts_entry = manage(new Gtk::SpinButton());
  interval_entry = manage(new Gtk::SpinButton());

  port_entry->set_range(1024, 65535);
  port_entry->set_increments(1, 10);
  port_entry->set_numeric(true);
  
  attempts_entry->set_range(0, 100);
  attempts_entry->set_increments(1, 10);
  attempts_entry->set_numeric(true);

  interval_entry->set_range(1, 3600);
  interval_entry->set_increments(1, 10);
  interval_entry->set_numeric(true);

  Gtk::Label *port_label = manage(new Gtk::Label("Server port"));
  Gtk::Label *attempts_label = manage(new Gtk::Label("Reconnect atttempts"));
  Gtk::Label *interval_label = manage(new Gtk::Label("Reconnect interval"));

  
  Gtk::Table *advanced_table = manage(new Gtk::Table(3, 2, false));
  advanced_table->set_row_spacings(2);
  advanced_table->set_col_spacings(6);
  advanced_table->set_border_width(6);
  int y = 0;
  advanced_table->attach(*port_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  advanced_table->attach(*port_entry, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  y++;
  advanced_table->attach(*attempts_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  advanced_table->attach(*attempts_entry, 1, 2, y, y+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);
  y++;
  advanced_table->attach(*interval_label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  advanced_table->attach(*interval_entry, 1, 2, y, y+1, Gtk::SHRINK | Gtk::FILL, Gtk::SHRINK);

  advanced_frame->add(*advanced_table);
  
  gp->pack_start(*advanced_frame, false, false, 0);

  box->show_all();
  tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*gp, *box));

  port_entry->signal_changed().connect(SigC::slot(*this, &CollectivePreferencePage::on_port_changed));
  interval_entry->signal_changed().connect(SigC::slot(*this, &CollectivePreferencePage::on_interval_changed));
  attempts_entry->signal_changed().connect(SigC::slot(*this, &CollectivePreferencePage::on_attempts_changed));
  
}


void
CollectivePreferencePage::create_peers_page(Gtk::Notebook *tnotebook)
{
  Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
  Gtk::Label *lab = manage(new Gtk::Label("Peers"));
  // Gtk::Image *img = manage(new Gtk::Image();
  // box->pack_start(*img, false, false, 0);
  box->pack_start(*lab, false, false, 0);

  Gtk::VBox *gp = manage(new Gtk::VBox(false, 6));
  gp->set_border_width(6);

  // Identity
  Gtk::Frame *peers_frame = new Gtk::Frame("Peers");
  peers_frame->set_border_width(6);

  gp->pack_start(*peers_frame, false, false, 0);

  box->show_all();
  tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*gp, *box));
  
}


void
CollectivePreferencePage::init_page_values()
{
  Configurator *c = GUIControl::get_instance()->get_configurator();
  bool is_set;

  // Master enabled switch.
  bool enabled = false;
  is_set = c->get_value(DistributionManager::CFG_KEY_DISTRIBUTION + DistributionManager::CFG_KEY_DISTRIBUTION_ENABLED, &enabled);
  if (!is_set)
    {
      enabled = false;
    }
  
  enabled_cb->set_active(enabled);

  // Username.
  string str;
  is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_USERNAME, &str);
  if (!is_set)
    {
      str = "";
    }
  
  username_entry->set_text(str);

  // Password
  is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PASSWORD, &str);
  if (!is_set)
    {
      str = "";
    }
  
  password1_entry->set_text(str);
  password2_entry->set_text(str);

  // Port
  int value;
  is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PORT, &value);
  if (!is_set)
    {
      value = DEFAULT_PORT;
    }
  
  port_entry->set_value(value);

  // Attempts
  is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS, &value);
  if (!is_set)
    {
      value = DEFAULT_ATTEMPTS;
    }
  
  attempts_entry->set_value(value);

  // Interval
  is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_INTERVAL, &value);
  if (!is_set)
    {
      value = DEFAULT_INTERVAL;
    }
  
  interval_entry->set_value(value);
}


void
CollectivePreferencePage::on_enabled_toggled()
{
  TRACE_ENTER("CollectivePreferencePage::on_enabled_toggled");
  bool enabled = enabled_cb->get_active();
  Configurator *c = GUIControl::get_instance()->get_configurator();
  c->set_value(DistributionManager::CFG_KEY_DISTRIBUTION + DistributionManager::CFG_KEY_DISTRIBUTION_ENABLED, enabled);
  TRACE_EXIT();
}


void
CollectivePreferencePage::on_username_changed()
{
  string name = username_entry->get_text();
  Configurator *c = GUIControl::get_instance()->get_configurator();
  c->set_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_USERNAME, name);
}


void
CollectivePreferencePage::on_password_changed()
{
  string pw1 = password1_entry->get_text();
  string pw2 = password2_entry->get_text();

  if (pw1 == pw2)
    {
      Configurator *c = GUIControl::get_instance()->get_configurator();
      c->set_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PASSWORD, pw1);
    }
}


void
CollectivePreferencePage::on_port_changed()
{
  int value = (int) port_entry->get_value();
  Configurator *c = GUIControl::get_instance()->get_configurator();
  c->set_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PORT, value);
}


void
CollectivePreferencePage::on_interval_changed()
{
  int value = (int) interval_entry->get_value();
  Configurator *c = GUIControl::get_instance()->get_configurator();
  c->set_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_INTERVAL, value);
}


void
CollectivePreferencePage::on_attempts_changed()
{
  int value = (int) attempts_entry->get_value();
  Configurator *c = GUIControl::get_instance()->get_configurator();
  c->set_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS, value);
}
