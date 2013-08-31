// RestBreakWindow.cc --- window for the microbreak
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

const int TIMEOUT = 1000;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "RestBreakWindow.hh"

#include "preinclude.h"

#include "debug.hh"
#include "nls.h"

#include "Text.hh"
#include "TimeBar.hh"
#include "Util.hh"
#include "UiUtil.hh"

#include "CoreFactory.hh"

#include "Exercise.hh"
#include "ExercisesPanel.hh"


IBreakWindow::Ptr
RestBreakWindow::create(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode)
{
  return Ptr(new RestBreakWindow(screen, break_flags, mode));
}

//! Constructor
RestBreakWindow::RestBreakWindow(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode)
  : BreakWindow(screen, BREAK_ID_REST_BREAK, break_flags, mode),
    timebar(NULL),
    progress_value(0),
    progress_max_value(0)
{
  TRACE_ENTER("RestBreakWindow::RestBreakWindow");
  setWindowTitle(_("Rest break"));
  TRACE_EXIT();
}

QWidget *
RestBreakWindow::create_gui()
{
  // Add other widgets.
  QVBoxLayout *box = new QVBoxLayout;
  
  pluggable_panel = new QHBoxLayout;
  box->addLayout(pluggable_panel);

  timebar = new TimeBar;
  box->addWidget(timebar);
  
  QHBoxLayout *button_box = create_break_buttons(true, false);
  if (button_box != NULL)
    {
      box->addLayout(button_box);
    }

  QWidget *widget = new QWidget;
  widget->setLayout(box);

  return widget;
}


//! Destructor.
RestBreakWindow::~RestBreakWindow()
{
  TRACE_ENTER("RestBreakWindow::~RestBreakWindow");
  TRACE_EXIT();
}


//! Starts the restbreak.
void
RestBreakWindow::start()
{
  TRACE_ENTER("RestBreakWindow::start");

  if (get_exercise_count() > 0)
    {
      install_exercises_panel();
    }
  else
    {
       install_info_panel();
    }

  BreakWindow::start();

  TRACE_EXIT();
}


//! Period timer callback.
void
RestBreakWindow::update_break_window()
{
  draw_time_bar();
}


void
RestBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}


//! Draws the timer bar.
void
RestBreakWindow::draw_time_bar()
{
  time_t time = progress_max_value - progress_value;
  char s[128];
  sprintf(s, _("Rest break for %s"), Text::time_to_string(time, true).c_str());

  timebar->set_progress(progress_value, progress_max_value);
  timebar->set_text(s);
  timebar->update();
}


QHBoxLayout *
RestBreakWindow::create_info_panel()
{
  // Label
  QLabel *lab = new QLabel;

  // Icon
  QLabel *image = new QLabel;
  std::string file = Util::complete_directory("rest-break.png", Util::SEARCH_PATH_IMAGES);
  image->setPixmap(QPixmap(file.c_str()));
  
  std::string txt;
  if (get_break_flags() & BREAK_FLAGS_NATURAL)
    {
      txt = UiUtil::create_alert_text
        (_("Natural rest break"),
         _("This is your natural rest break."));
    }
  else
    {
      txt = UiUtil::create_alert_text
        (_("Rest break"),
         _("This is your rest break. Make sure you stand up and\n"
           "walk away from your computer on a regular basis. Just\n"
           "walk around for a few minutes, stretch, and relax."));
    }

  lab->setText(txt.c_str());
  
  // HBox
  QHBoxLayout *box = new QHBoxLayout;
  box->addWidget(image);
  box->addWidget(lab);

  return box;
}

void
RestBreakWindow::clear_pluggable_panel()
{
  TRACE_ENTER("RestBreakWindow::clear_pluggable_panel");
  UiUtil::clear_layout(pluggable_panel);
  TRACE_EXIT();
}

int
RestBreakWindow::get_exercise_count()
{
  int ret = 0;

  if (Exercise::has_exercises())
    {
      ret = GUIConfig::get_number_of_exercises(BREAK_ID_REST_BREAK);
    }
  return ret;
}

void
RestBreakWindow::install_exercises_panel()
{
  if (get_screen() != 0 || (get_break_flags() & BREAK_FLAGS_NO_EXERCISES))
    {
      install_info_panel();
    }
  else
    {
      set_ignore_activity(true);
      clear_pluggable_panel();
      
      ExercisesPanel *exercises_panel = new ExercisesPanel(NULL);
      
      pluggable_panel->addWidget(exercises_panel);
      
      exercises_panel->set_exercise_count(get_exercise_count());
      exercises_panel->signal_stop().connect(boost::bind(&RestBreakWindow::install_info_panel, this)); 
      
    }
}

void
RestBreakWindow::install_info_panel()
{
  // Gtk::Requisition old_size;
  // Gtk::Requisition natural_size;
  // get_preferred_size(old_size, natural_size);

  set_ignore_activity(false);

  clear_pluggable_panel();
  pluggable_panel->addLayout(create_info_panel());

  pluggable_panel->invalidate();
  QWidget *w = pluggable_panel->parentWidget();
  while (w)
    {
      qDebug() << "b: " << w->size();
      w->adjustSize(); 
      qDebug() << "a: " << w->size();
      w = w->parentWidget();
    }

  center();
  
  // GUIConfig::BlockMode block_mode = GUIConfig::get_block_mode();
  // if (block_mode == GUIConfig::BLOCK_MODE_NONE &&
  //     screen == 0)
  //   {
  //     Gtk::Requisition new_size;
  //     get_preferred_size(new_size, natural_size);

  //     int width_delta = (new_size.width - old_size.width) / 2;
  //     int height_delta = (new_size.height -  old_size.height) / 2;

  //     int x, y;
  //     get_position(x, y);
  //     move(x - width_delta, y - height_delta);
  //   }
  // else
    // {
    //   center();
    // }
}


void
RestBreakWindow::set_ignore_activity(bool i)
{
  ICore::Ptr core = CoreFactory::get_core();

// #ifdef PLATFORM_OS_WIN32
//   if( W32ForceFocus::GetForceFocusValue() )
//     {
//       i = true;
//     }
// #endif

    core->set_insist_policy(i ?
                            ICore::INSIST_POLICY_IGNORE :
                            ICore::INSIST_POLICY_HALT);
}
