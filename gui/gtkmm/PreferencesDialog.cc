// PreferencesDialog.cc --- Preferences dialog
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

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <assert.h>

#include "PreferencesDialog.hh"
#include "TimerPreferencesPanel.hh"
#include "Configurator.hh"
#include "ControlInterface.hh"
#include "SoundPlayer.hh"
#include "TimeEntry.hh"
#include "Util.hh"
#include "MainWindow.hh"

#ifdef HAVE_DISTRIBUTION
#include "NetworkPreferencePage.hh"
#endif

struct MonitorPreset
{
  const char *name;
  int activity;
  int idle;
  int noise;
};

static MonitorPreset presets[] =
{
  { "Trigger-happy", 0, 1000, 0 },
  { "Quick", 500, 3000, 20000 },
  { "Normal", 1000, 5000, 10000 },
  { "Sluggish", 5000, 10000, 4000 },
  { "Numb", 10000, 10000, 9000 },
  { NULL, 0, 0, 0 }, 
  { "Custom settings", -1, -1, -1 },
};
    


PreferencesDialog::PreferencesDialog()
  : Gtk::Dialog("Preferences", true, false)
{
  TRACE_ENTER("PreferencesDialog::PreferencesDialog");

  // Pages
  Gtk::Widget *monitor_page = manage(create_monitor_page());
  Gtk::Widget *gui_page = manage(create_gui_page());
  Gtk::Widget *timer_page = manage(create_timer_page());
#ifdef HAVE_DISTRIBUTION
  Gtk::Widget *network_page = manage(create_network_page());
#endif
  
  // Notebook
  Gtk::Notebook *notebook = manage(new Gtk::Notebook());
  notebook->set_tab_pos (Gtk::POS_TOP);  
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*timer_page, _("Timers")));
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*monitor_page, _("Monitoring")));
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*gui_page, _("User interface")));
#ifdef HAVE_DISTRIBUTION
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*network_page, _("Network")));
#endif
  
  // Dialog
  get_vbox()->pack_start(*notebook, false, false, 0);
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  show_all();


  TRACE_EXIT();
}


//! Destructor.
PreferencesDialog::~PreferencesDialog()
{
  TRACE_ENTER("PreferencesDialog::~PreferencesDialog");
  TRACE_EXIT();
}



Gtk::Widget *
PreferencesDialog::create_gui_page()
{
  // Always-on-top
  ontop_cb = manage
    (new Gtk::CheckButton
     (_("The main window stays always on top of other windows")));
  ontop_cb->signal_toggled().connect(SigC::slot(*this, &PreferencesDialog::on_always_on_top_toggled));
  ontop_cb->set_active(MainWindow::get_always_on_top());

  // Sound types
  sound_button  = manage(new Gtk::OptionMenu());
  Gtk::Menu *sound_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &sound_list = sound_menu->items();
  sound_button->set_menu(*sound_menu);
  sound_list.push_back(Gtk::Menu_Helpers::MenuElem(_("No sounds")));
  sound_list.push_back(Gtk::Menu_Helpers::MenuElem
                       (_("Play sounds using sound card")));
  sound_list.push_back(Gtk::Menu_Helpers::MenuElem
                       (_("Play sounds using built-in speaker")));
  int idx;
  if (! SoundPlayer::is_enabled())
    idx = 0;
  else
    {
      if (SoundPlayer::DEVICE_SPEAKER == SoundPlayer::get_device())
        idx = 2;
      else
        idx = 1;
    }
  sound_button->set_history(idx);
  sound_button->signal_changed().connect(SigC::slot(*this, &PreferencesDialog::on_sound_changed));
  
  
#ifdef WIN32
  // Tray start
  win32_start_in_tray_cb
    = manage(new Gtk::CheckButton(_("Hide main window at start-up")));
  win32_start_in_tray_cb->signal_toggled()
    .connect(SigC::slot(*this,
			&PreferencesDialog::win32_on_start_in_tray_toggled));
  win32_start_in_tray_cb->set_active(MainWindow::win32_get_start_in_tray());
#endif

  // Options
  Gtk::VBox *opts_box = new Gtk::VBox(false, 0);
  opts_box->pack_start(*ontop_cb, false, false, 0);
#ifdef WIN32
  opts_box->pack_start(*win32_start_in_tray_cb, false, false, 0);
#endif
  opts_box->pack_start(*sound_button, false, false, 0);
  opts_box->set_border_width(6);

  // Page
  Gtk::VBox *gui_page
    = create_page
    (_("You can configure the user interface related settings from here."),
     "display.png");
  Gtk::Frame *gui_frame = manage(new Gtk::Frame("Options"));
  gui_frame->add(*opts_box);
  gui_page->pack_start(*gui_frame, false, false, 0);

  return gui_page;
}

Gtk::Widget *
PreferencesDialog::create_timer_page()
{
  // Timers page
  Gtk::VBox *timer_page
    = create_page
    (_("This dialog allows you to change the settings of the timers.  Each unit\n"
     "of time is broken down into hours, minutes and seconds (also known as\n"
     "the \"hh:mm:ss\" format).  These can all be controlled individually."),
     "time.png");
  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos (Gtk::POS_TOP);  
  for (int i = 0; i < GUIControl::TIMER_ID_SIZEOF; i++)
    {
      // Label
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[i];
      
      Gtk::HBox *box = manage(new Gtk::HBox(false, 3));
      Gtk::Label *lab = manage(new Gtk::Label(_(timer->label)));
      Gtk::Image *img = manage(new Gtk::Image(timer->icon));
      box->pack_start(*img, false, false, 0);
      box->pack_start(*lab, false, false, 0);

      TimerPreferencesPanel *tp = manage(new TimerPreferencesPanel(GUIControl::TimerId(i)));
      box->show_all();
      tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*tp, *box));
    }
  timer_page->pack_start(*tnotebook, false, false, 0);

  return timer_page;
}

Gtk::Widget *
PreferencesDialog::create_monitor_page()
{
  // Monitor preset
  monitor_preset_button  = manage(new Gtk::OptionMenu());
  Gtk::Menu *monitor_preset_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &preset_list = monitor_preset_menu->items();
  monitor_preset_button->set_menu(*monitor_preset_menu);
  for (int i = 0; i < sizeof(presets)/sizeof(presets[0]); i++)
    {
      MonitorPreset *mp = presets + i;
      if (! mp->name)
        preset_list.push_back(Gtk::Menu_Helpers::SeparatorElem());
      else
        preset_list.push_back(Gtk::Menu_Helpers::MenuElem(_(mp->name)));
    }

  Gtk::HBox *mon_pbox = manage(new Gtk::HBox(false, 6));
  mon_pbox->set_border_width(6);
  Gtk::Label *label = manage(new Gtk::Label(_("Monitoring preset")));
  mon_pbox->pack_start(*label, false, false, 0);
  mon_pbox->pack_start(*monitor_preset_button, false, false, 0);

  Gtk::Frame *mon_pframe = manage(new Gtk::Frame(_("Preset")));
  mon_pframe->add(*mon_pbox);

  // Monitor table
  Gtk::Table *mon_table = manage(new Gtk::Table(4, 3, false));
  mon_table->set_row_spacings(2);
  mon_table->set_col_spacings(6);
  mon_table->set_border_width(6);
  int y = 0;
  
  label = manage(new Gtk::Label(_("Activity time (ms)")));
  activity_time = manage(new TimeEntry(true));
  mon_table->attach(*label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  mon_table->attach(*activity_time, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  int val;
  GUIControl::get_instance()->get_configurator()
    ->get_value(ControlInterface::CFG_KEY_MONITOR_ACTIVITY, &val);
  activity_time->set_value(val);
  y++;

  label = manage(new Gtk::Label(_("Noise time (ms)")));
  noise_time = manage(new TimeEntry(true));
  mon_table->attach(*label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  mon_table->attach(*noise_time, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  GUIControl::get_instance()->get_configurator()
    ->get_value(ControlInterface::CFG_KEY_MONITOR_NOISE, &val);
  noise_time->set_value(val);
  y++;
  
  label = manage(new Gtk::Label(_("Idle time (ms)")));
  idle_time = manage(new TimeEntry(true));
  mon_table->attach(*label, 0, 1, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  mon_table->attach(*idle_time, 1, 2, y, y+1, Gtk::SHRINK, Gtk::SHRINK);
  GUIControl::get_instance()->get_configurator()
    ->get_value(ControlInterface::CFG_KEY_MONITOR_IDLE, &val);
  idle_time->set_value(val);
  y++;

  Gtk::VSeparator *sep = manage(new Gtk::VSeparator());
  label = manage
    (new Gtk::Label
     (_("The timers are started if, during the 'Activity\n"
        "time', there was no inactivitiy for longer than\n"
        "'Noise time'.  When 'Noise time' exceeds 'Activity\n"
        "time', the timers are started if the time between\n"
        "two events is greater then 'Activity time' but\n"
        "smaller then 'Noise time'.")
      ));
  mon_table->attach(*sep, 2, 3, 0, 3, Gtk::SHRINK, Gtk::FILL);
  mon_table->attach(*label, 3, 4, 0, 3, Gtk::SHRINK, Gtk::SHRINK);

  update_preset();

  // Signals
  activity_time->signal_value_changed().connect(SigC::slot(*this, &PreferencesDialog::on_activity_time_changed));
  idle_time->signal_value_changed().connect(SigC::slot(*this, &PreferencesDialog::on_idle_time_changed));
  monitor_preset_button->signal_changed().connect(SigC::slot(*this, &PreferencesDialog::on_monitor_preset_changed));
  noise_time->signal_value_changed().connect(SigC::slot(*this, &PreferencesDialog::on_noise_time_changed));
  
  // Monitor frame
  Gtk::Frame *mon_tframe = manage(new Gtk::Frame(_("Custom settings")));
  mon_tframe->add(*mon_table);

  // Monitoring page
  Gtk::VBox *monitor_page
    = create_page
    (_("Activity and idle time detection can be fine-tuned by the monitor\n"
     "settings.  You can choose from various presets, or define your own\n"
     "custom settings."),
     "monitoring.png");
  monitor_page->pack_start(*mon_pframe, false, false, 0);
  monitor_page->pack_start(*mon_tframe, false, false, 0);
  return monitor_page;
}

#ifdef HAVE_DISTRIBUTION
Gtk::Widget *
PreferencesDialog::create_network_page()
{
  // Timers page
  Gtk::VBox *network_page
    = create_page
    (_("You can connect several instances of Workrave in a network. All connected\n"
       "instances share the same timer information, meaning you will be reminded\n"
       "of your breaks even if you switch computers."),
     "network.png");

  Gtk::Widget *page = manage(new NetworkPreferencePage());
  
  network_page->pack_start(*page, true, true, 0);

  return network_page;
}
#endif

Gtk::VBox *
PreferencesDialog::create_page(const char *label, const char *image)
{
  Gtk::HBox *info_box = manage(new Gtk::HBox(false));
  string icon = Util::complete_directory(image, Util::SEARCH_PATH_IMAGES);
  Gtk::Image *info_img = manage(new Gtk::Image(icon));
  Gtk::Label *info_lab = manage(new Gtk::Label(label));
  info_box->pack_start(*info_img, false, false, 6);
  info_box->pack_start(*info_lab, false, true, 6);

  Gtk::VBox *page = new Gtk::VBox(false, 6);
  page->pack_start(*info_box, false, true, 0);
  page->set_border_width(6);
  return page;
}

void
PreferencesDialog::on_always_on_top_toggled()
{
  MainWindow::set_always_on_top(ontop_cb->get_active());
}

void
PreferencesDialog::on_sound_changed()
{
  int idx = sound_button->get_history();
  SoundPlayer::set_enabled(idx > 0);
  if (idx > 0)
    {
      SoundPlayer::Device dev = idx == 1
        ? SoundPlayer::DEVICE_SOUNDCARD
        : SoundPlayer::DEVICE_SPEAKER;
      SoundPlayer::set_device(dev);
    }
}

#ifdef WIN32
void
PreferencesDialog::win32_on_start_in_tray_toggled()
{
  MainWindow::win32_set_start_in_tray(win32_start_in_tray_cb->get_active());
}
#endif

void
PreferencesDialog::update_preset()
{
  int noise, idle, activity;
  Configurator *cfg = GUIControl::get_instance()->get_configurator();
  cfg->get_value(ControlInterface::CFG_KEY_MONITOR_NOISE, &noise);
  cfg->get_value(ControlInterface::CFG_KEY_MONITOR_ACTIVITY, &activity);
  cfg->get_value(ControlInterface::CFG_KEY_MONITOR_IDLE, &idle);

  int match_idx = -1;
  for (int i = 0; i < sizeof(presets)/sizeof(presets[0]); i++)
    {
      MonitorPreset *p = &presets[i];
      if (! p->name)
        continue;
      
      if (p->idle == idle
          && p->activity == activity
          && p->noise == noise)
        {
          match_idx = i;
          break;
        }
      else if (p->idle < 0)
        {
          // Custom
          match_idx = i;
        }
    }
  assert(match_idx >= 0);
  monitor_preset_button->set_history(match_idx);
}

void
PreferencesDialog::on_activity_time_changed()
{
  set_activity_time(activity_time->get_value());
  update_preset();
}

void
PreferencesDialog::on_idle_time_changed()
{
  set_idle_time(idle_time->get_value());
  update_preset();
}

void
PreferencesDialog::on_monitor_preset_changed()
{
  int idx = monitor_preset_button->get_history();
  MonitorPreset *p = &presets[idx];
  if (p->activity >= 0)
    {
      noise_time->set_value(p->noise);
      activity_time->set_value(p->activity);
      idle_time->set_value(p->idle);
      set_activity_time(p->activity);
      set_noise_time(p->noise);
      set_idle_time(p->idle);
    }
}

void
PreferencesDialog::on_noise_time_changed()
{
  set_noise_time(noise_time->get_value());
  update_preset();
}

void
PreferencesDialog::set_noise_time(time_t t)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(ControlInterface::CFG_KEY_MONITOR_NOISE, t);
}

void
PreferencesDialog::set_activity_time(time_t t)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(ControlInterface::CFG_KEY_MONITOR_ACTIVITY, t);
}

void
PreferencesDialog::set_idle_time(time_t t)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(ControlInterface::CFG_KEY_MONITOR_IDLE, t);
}

int
PreferencesDialog::run()
{
  int id = Gtk::Dialog::run();
  GUIControl::get_instance()->get_configurator()->save();

  //FIXME: no return value. Raymond ???
}
