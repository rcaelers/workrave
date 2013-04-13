// BreakWindow.cc --- base class for the break windows
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BreakWindow.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QtGui>

#include <QStyle>
#include <QDesktopWidget>
#include <QApplication>

#include "debug.hh"
#include "nls.h"

// #include "config/IConfigurator.hh"

// #include "GUI.hh"
// #include "IBreak.hh"
// #include "GtkUtil.hh"
// #include "WindowHints.hh"
// #include "Frame.hh"
#include "System.hh"
#include "Util.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

// #if defined(PLATFORM_OS_WIN32)
// #include "DesktopWindow.hh"
// #elif defined(PLATFORM_OS_UNIX)
// #include "desktop-window.h"
// #endif

using namespace workrave;

//! Constructor
/*!
 *  \param control The controller.
 */
BreakWindow::BreakWindow(int screen,
                         BreakId break_id,
                         BreakFlags break_flags,
                         GUIConfig::BlockMode mode)
  : QWidget(0, Qt::Window
            | Qt::WindowStaysOnTopHint 
            | Qt::X11BypassWindowManagerHint 
            | Qt::FramelessWindowHint
            | Qt::WindowDoesNotAcceptFocus),
    break_id(break_id),
    screen(screen),
    block_mode(mode),
    break_flags(break_flags),
    frame(NULL),
    gui(NULL),
  postpone_button(NULL),
  skip_button(NULL),
  lock_button(NULL),
  shutdown_button(NULL)
{
  TRACE_ENTER("BreakWindow::BreakWindow");
  init();
  TRACE_EXIT();
}


//! Init GUI
void
BreakWindow::init()
{
  gui = create_gui();

  // if (mode != GUIConfig::BLOCK_MODE_NONE)
  //   {
  //     // Disable titlebar to appear like a popup
  //     set_decorated(false);
  //     set_skip_taskbar_hint(true);
  //     set_skip_pager_hint(true);
  //     window->set_functions(Gdk::FUNC_MOVE);
  //   }

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  if (block_mode == GUIConfig::BLOCK_MODE_NONE)
    {
      layout->addWidget(gui);
    }
  else
    {
      frame = new Frame;
      frame->set_frame_style(Frame::STYLE_SOLID);
      frame->set_frame_width(6, 6);

      QVBoxLayout *frameLayout = new QVBoxLayout;
      frame->setLayout(frameLayout);
      frameLayout->addWidget(gui);

      layout->addWidget(frame);
    }
}


//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");

  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }

  TRACE_EXIT();
}


//! Centers the window.
void
BreakWindow::center()
{
  QDesktopWidget *dw = QApplication::desktop();
  const QRect	rect = dw->screenGeometry(screen);
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), rect));
}



//! Creates the lock button
QAbstractButton *
BreakWindow::create_lock_button()
{
  QPushButton *button = NULL;
  if (System::is_lockable())
    {
      std::string file = Util::complete_directory("lock.png", Util::SEARCH_PATH_IMAGES);
      QPixmap pixmap(file.c_str());
      QIcon icon(pixmap);

      button = new QPushButton;	
      button->setIcon(icon);
      button->setIconSize(pixmap.rect().size());

      connect(button, &QPushButton::click, this, &BreakWindow::on_lock_button_clicked);
    }
  return button;
}

//! Creates the lock button
QAbstractButton *
BreakWindow::create_shutdown_button()
{
  QPushButton *button = NULL;
  if (System::is_shutdown_supported())
    {
      std::string file = Util::complete_directory("shutdown.png", Util::SEARCH_PATH_IMAGES);
      QPixmap pixmap(file.c_str());
      QIcon icon(pixmap);

      button = new QPushButton(_("Shut _down"));
      button->setIcon(icon);
      button->setIconSize(pixmap.rect().size());

      connect(button, &QPushButton::click, this, &BreakWindow::on_shutdown_button_clicked);
    }
  return button;
}

//! Creates the skip button.
QAbstractButton *
BreakWindow::create_skip_button()
{
  QPushButton *button = new QPushButton(_("_Skip"));
  connect(button, &QPushButton::click, this, &BreakWindow::on_skip_button_clicked);
  return button;
}


//! Creates the postpone button.
QAbstractButton *
BreakWindow::create_postpone_button()
{
  QPushButton *button = new QPushButton(_("_Postpone"));
  connect(button, &QPushButton::click, this, &BreakWindow::on_postpone_button_clicked);
  return button;
}


//! The lock button was clicked.
void
BreakWindow::on_lock_button_clicked()
{
  if (System::is_lockable())
    {
      // IGUI *gui = GUI::get_instance();
      // gui->interrupt_grab();
      System::lock();
    }
}

//! The lock button was clicked.
void
BreakWindow::on_shutdown_button_clicked()
{
  // IGUI *gui = GUI::get_instance();
  // gui->interrupt_grab();

  System::shutdown();
}


//! User has closed the main window.
// bool
// BreakWindow::on_delete_event(GdkEventAny *)
// {
//   if (block_mode == GUIConfig::BLOCK_MODE_NONE)
//     {
//       on_postpone_button_clicked();
//     }
//   return TRUE;
// }


//! The postpone button was clicked.
void
BreakWindow::on_postpone_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr b = core->get_break(break_id);
  
  b->postpone_break();
  resume_non_ignorable_break();

  TRACE_EXIT();
}



//! The skip button was clicked.
void
BreakWindow::on_skip_button_clicked()
{
  TRACE_ENTER("BreakWindow::on_postpone_button_clicked");
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr b = core->get_break(break_id);

  b->skip_break();
  resume_non_ignorable_break();

  TRACE_EXIT();
}


void
BreakWindow::resume_non_ignorable_break()
{
  TRACE_ENTER("BreakWindow::resume_non_ignorable_break");
  ICore::Ptr core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();

  TRACE_MSG("break flags " << break_flags);
  
  if (! (break_flags & BreakWindow::BREAK_FLAGS_USER_INITIATED) &&
      mode == OPERATION_MODE_NORMAL)
    {
      for (int id = break_id - 1; id >= 0; id--)
        {
          TRACE_MSG("Break " << id << ": check ignorable");

          bool ignorable = GUIConfig::get_ignorable(BreakId(id));
          if (!ignorable)
            {
              TRACE_MSG("Break " << id << " not ignorable");

              ICore::Ptr core = CoreFactory::get_core();
              IBreak::Ptr b = core->get_break(BreakId(id));

              if (b->get_elapsed_time() > b->get_limit())
                {
                  TRACE_MSG("Break " << id << " not ignorable and overdue");

                  core->force_break(BreakId(id), BREAK_HINT_NONE);
                  break;
                }
            }
        }
    }
}

//! Control buttons.
QHBoxLayout *
BreakWindow::create_break_buttons(bool lockable,
                                  bool shutdownable)
{
  QHBoxLayout *box = NULL;

  if ((break_flags != BREAK_FLAGS_NONE) || lockable || shutdownable)
    {
      box = new QHBoxLayout;

      if (shutdownable)
        {
          shutdown_button = create_shutdown_button();
          if (shutdown_button != NULL)
            {
              box->addWidget(shutdown_button);
            }
        }

      if (lockable)
        {
          lock_button = create_lock_button();
          if (lock_button != NULL)
            {
              box->addWidget(lock_button);
            }
        }

      if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
        {
          skip_button = create_skip_button();
          box->addWidget(skip_button);
        }

      if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
        {
          postpone_button = create_postpone_button();
          box->addWidget(postpone_button);
        }
    }

  return box;
}


//! Starts the daily limit.
void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");

  update_break_window();
  show();
  center();

  // Set window hints.
  // set_skip_pager_hint(true);
  // set_skip_taskbar_hint(true);
  // WindowHints::set_always_on_top(this, true);
  // raise();

  TRACE_EXIT();
}


//! Stops the daily limit.
void
BreakWindow::stop()
{
  TRACE_ENTER("BreakWindow::stop");

  if (frame != NULL)
    {
      frame->set_frame_flashing(0);
    }

  hide();
  TRACE_EXIT();
}


//! Refresh
void
BreakWindow::refresh()
{
  TRACE_ENTER("BreakWindow::refresh");
  update_break_window();
  TRACE_EXIT();
}

void
BreakWindow::update_break_window()
{
}
