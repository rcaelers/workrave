// PreferencesDialog.hh --- Statistics Dialog
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
//

#ifndef STATISTICSDIALOG_HH
#define STATISTICSDIALOG_HH

#include <stdio.h>

#include "preinclude.h"

#include <gtkmm.h>

class StatisticsDialog : public Gtk::Dialog
{
public:  
  StatisticsDialog();
  ~StatisticsDialog();

  int run();
  
private:
};

#endif // STATISTICSWINDOW_HH
