// PreferencesDialog.hh --- Statistics Dialog
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#include "GUIControl.hh"
#include "Statistics.hh"

#include <gtkmm.h>

class StatisticsDialog : public Gtk::Dialog
{
public:  
  StatisticsDialog();
  ~StatisticsDialog();

  static const int BREAK_STATS = 7;
  
  int run();
  
private:
  /** Stats */
  Statistics *statistics;
  
  /** Labels for break stats. */
  Gtk::Label *break_labels[GUIControl::BREAK_ID_SIZEOF][9];

  /** Labels for break stats. */
  Gtk::Label *activity_labels[5];
  
  /** Labels for break stats. */
  Gtk::Label *daily_usage_label;

  /** Day selection adjustment. */
  Gtk::Adjustment *day_adjust;

  /** Labels indicating the start time of the visible data. */
  Gtk::Label *start_time_label;

  /** Labels indicating the end time of the visible data. */
  Gtk::Label *end_time_label;

  /** */
  Gtk::Tooltips *tips;
  
  void init_gui();
  void init_page_values();
  void select_day(int day);
  
  void create_info_box(Gtk::Box *box);
  void create_break_page(Gtk::Notebook *tnotebook);
  void create_activity_page(Gtk::Notebook *tnotebook);

  void attach_left(Gtk::Table &table, Widget &child, guint left_attach, guint top_attach);
  void attach_right(Gtk::Table &table, Widget &child, guint left_attach, guint top_attach);
  Gtk::Widget *createLabel(char *label, char *tooltip);

  void on_scrollbar();

  bool on_focus_in_event(GdkEventFocus *event);
  bool on_focus_out_event(GdkEventFocus *event);  
};

#endif // STATISTICSWINDOW_HH
