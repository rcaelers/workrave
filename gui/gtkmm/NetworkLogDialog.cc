// NetworkLogDialog.cc --- NetworkLog dialog
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

#include "NetworkLogDialog.hh"

#include "DistributionManager.hh"
#include "GUIControl.hh"
#include "Util.hh"

NetworkLogDialog::NetworkLogDialog()
  : Gtk::Dialog(_("Network log"), false, true)
{
  TRACE_ENTER("NetworkLogDialog::NetworkLogDialog");

  set_default_size(600, 400);

  text_buffer = Gtk::TextBuffer::create();

  text_view = Gtk::manage(new Gtk::TextView(text_buffer));
  text_view->set_cursor_visible(false);
  text_view->set_editable(false);
  
  scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled_window.add(*text_view);

  Gtk::HBox *box = manage(new Gtk::HBox(false, 6));
  box->pack_start(scrolled_window, true, true, 0);

  get_vbox()->pack_start(*box, true, true, 0);

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  show_all();

  TRACE_EXIT();
}


//! Destructor.
NetworkLogDialog::~NetworkLogDialog()
{
  TRACE_ENTER("NetworkLogDialog::~NetworkLogDialog");
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->remove_log_listener(this);
    }
  TRACE_EXIT();
}


void
NetworkLogDialog::distribution_log(string msg)
{
  Gtk::TextIter iter = text_buffer->end();
  iter = text_buffer->insert(iter, msg);
  Gtk::Adjustment *a = scrolled_window.get_vadjustment();
  a->set_value(a->get_upper());
}

void
NetworkLogDialog::init()
{
  DistributionManager *dist_manager = DistributionManager::get_instance();

  Gtk::TextIter iter = text_buffer->end();

  if (dist_manager != NULL)
    {
      list<string> logs = dist_manager->get_logs();

      for (list<string>::iterator i = logs.begin(); i != logs.end(); i++)
        {
          iter = text_buffer->insert(iter, (*i));
        }

      dist_manager->add_log_listener(this);
      Gtk::Adjustment *a = scrolled_window.get_vadjustment();
      a->set_value(a->get_upper());
    }
}

int
NetworkLogDialog::run()
{
  TRACE_ENTER("NetworkLogDialog::run")
  init();
  
  show_all();
  TRACE_EXIT();
}


void
NetworkLogDialog::on_response(int response)
{
  TRACE_ENTER("NetworkLogDialog::on_response")
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->remove_log_listener(this);
    }
  TRACE_EXIT();
}

