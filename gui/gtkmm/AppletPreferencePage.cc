// AppletPreferencePage.cc --- Preferences widgets for a timer
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

#include <sstream>

#include <unistd.h>
#include "GUIControl.hh"
#include "AppletPreferencePage.hh"
#include "Configurator.hh"
#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"

AppletPreferencePage::AppletPreferencePage()
  : Gtk::HBox(false, 6)
{
  TRACE_ENTER("AppletPreferencePage::AppletPreferencePage");

  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);  

  //init_page_values();

  // pack_start(*tnotebook, true, true, 0);

  TRACE_EXIT();
}


AppletPreferencePage::~AppletPreferencePage()
{
  TRACE_ENTER("AppletPreferencePage::~AppletPreferencePage");
  TRACE_EXIT();
}


// void
// AppletPreferencePage::init_page_values()
// {
//   Configurator *c = GUIControl::get_instance()->get_configurator();
//   bool is_set;

//   // Master enabled switch.
//   bool enabled = false;
//   is_set = c->get_value(DistributionManager::CFG_KEY_DISTRIBUTION + DistributionManager::CFG_KEY_DISTRIBUTION_ENABLED, &enabled);
//   if (!is_set)
//     {
//       enabled = false;
//     }
  
//   enabled_cb->set_active(enabled);

//   // Username.
//   string str;
//   is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_USERNAME, &str);
//   if (!is_set)
//     {
//       str = "";
//     }
  
//   username_entry->set_text(str);

//   // Password
//   is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PASSWORD, &str);
//   if (!is_set)
//     {
//       str = "";
//     }
  
//   password_entry->set_text(str);

//   // Port
//   int value;
//   is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PORT, &value);
//   if (!is_set)
//     {
//       value = DEFAULT_PORT;
//     }
  
//   port_entry->set_value(value);

//   // Attempts
//   is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS, &value);
//   if (!is_set)
//     {
//       value = DEFAULT_ATTEMPTS;
//     }
  
//   attempts_entry->set_value(value);

//   // Interval
//   is_set = c->get_value(DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP + DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_INTERVAL, &value);
//   if (!is_set)
//     {
//       value = DEFAULT_INTERVAL;
//     }
  
//   interval_entry->set_value(value);

//   // Peers
//   is_set = c->get_value(DistributionManager::CFG_KEY_DISTRIBUTION + DistributionManager::CFG_KEY_DISTRIBUTION_PEERS, &str);
//   if (!is_set)
//     {
//       str = "";
//     }

// }
