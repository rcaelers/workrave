// StatisticsDialog.cc --- Statistics dialog
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <unistd.h>
#include <assert.h>

#include "StatisticsDialog.hh"
#include "Util.hh"

StatisticsDialog::StatisticsDialog()
  : Gtk::Dialog("Statistics", true, false)
{
  TRACE_ENTER("StatisticsDialog::StatisticsDialog");

  // Dialog
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  show_all();

  TRACE_EXIT();
}


//! Destructor.
StatisticsDialog::~StatisticsDialog()
{
  TRACE_ENTER("StatisticsDialog::~StatisticsDialog");
  TRACE_EXIT();
}

int
StatisticsDialog::run()
{
  int id = Gtk::Dialog::run();
  //FIXME: what to return???
  return id;
}
