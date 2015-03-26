// PreludeWindow.cc
//
// Copyright (C) 2006, 2007, 2013 Raymond Penners & Rob Caelers
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

#include "PreludeWindow.hh"

#include <boost/format.hpp>
#include <boost/make_shared.hpp>

#include <QtGui>

#include <QStyle>
#include <QDesktopWidget>
#include <QApplication>

#include "IApp.hh"

#include "debug.hh"
#include "nls.h"
#include "Text.hh"
#include "utils/AssetPath.hh"



using namespace workrave;
using namespace workrave::utils;


IPreludeWindow::Ptr
PreludeWindow::create(int screen, workrave::BreakId break_id)
{
  return Ptr(new PreludeWindow(screen, break_id));
}


PreludeWindow::PreludeWindow(int screen, workrave::BreakId break_id)
  : QWidget(0, Qt::Window
            | Qt::WindowStaysOnTopHint
            | Qt::X11BypassWindowManagerHint
            | Qt::FramelessWindowHint
            | Qt::WindowDoesNotAcceptFocus),
    break_id(break_id),
    screen(screen),
    progress_value(0),
    progress_max_value(1),
    flash_visible(false),
    did_avoid(false)
{

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(1000);

  layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);

  timebar = new TimeBar;
  label = new QLabel;

  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      label->setText(_("Time for a micro-break?"));
      break;

    case BREAK_ID_REST_BREAK:
      label->setText(_("You need a rest break..."));
      break;

    case BREAK_ID_DAILY_LIMIT:
      label->setText(_("You should stop for today..."));
      break;

    default:
      break;
    }

  image = new QLabel;
  std::string file = AssetPath::complete_directory("prelude-hint.png", AssetPath::SEARCH_PATH_IMAGES);
  image->setPixmap(QPixmap(file.c_str()));

  frame = new Frame;
  frame->set_frame_style(Frame::STYLE_SOLID);
  frame->set_frame_width(6, 6);
  frame->set_frame_visible(false);
  connections.connect(frame->signal_flash(), boost::bind(&PreludeWindow::on_frame_flash, this, _1));

  QVBoxLayout *frameLayout = new QVBoxLayout;
  frame->setLayout(frameLayout);

  layout->addWidget(frame);

  QVBoxLayout *vlayout = new QVBoxLayout;
  vlayout->setSpacing(6);
  vlayout->addWidget(label);
  vlayout->addWidget(timebar);

  QHBoxLayout *hlayout = new QHBoxLayout;
  hlayout->setSpacing(6);

  hlayout->addWidget(image);
  hlayout->addLayout(vlayout);

  frameLayout->addLayout(hlayout);

  setLayout(layout);

  setAttribute(Qt::WA_Hover);
  setAttribute(Qt::WA_ShowWithoutActivating);

#ifdef PLATFORM_OS_OSX
  mouse_monitor = boost::make_shared<MouseMonitor>(boost::bind(&PreludeWindow::avoid_pointer, this, _1, _2));
#endif
}

PreludeWindow::~PreludeWindow()
{
}


//! Starts the microbreak.
void
PreludeWindow::start()
{
  TRACE_ENTER("PreludeWindow::start");

  timebar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);
  refresh();
  show();

  QDesktopWidget *dw = QApplication::desktop();
  const QRect	rect = dw->screenGeometry(screen);
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), rect));

#ifdef PLATFORM_OS_OSX
  mouse_monitor->start();
#endif

  raise();
  TRACE_EXIT();
}



//! Stops the microbreak.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");

  frame->set_frame_flashing(0);
  hide();

#ifdef PLATFORM_OS_OSX
  mouse_monitor->stop();
#endif

  TRACE_EXIT();
}


//! Refresh window.
void
PreludeWindow::refresh()
{
  std::string s;

  timebar->set_progress(progress_value, progress_max_value);

  int tminus = progress_max_value - progress_value;
  if (tminus >= 0 || (tminus < 0 && flash_visible))
    {
      if (tminus < 0)
        tminus = 0;

      s = boost::str(boost::format(progress_text) % Text::time_to_string(tminus));
    }
  timebar->set_text(s);
  timebar->update();

// #if defined(PLATFORM_OS_WIN32)
// // Vista GTK phantom toplevel parent kludge:
//   HWND hwnd = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
//   if( hwnd )
//     {
//       HWND hAncestor = GetAncestor( hwnd, GA_ROOT );
//       HWND hDesktop = GetDesktopWindow();
//       if( hAncestor && hDesktop && hAncestor != hDesktop )
//           hwnd = hAncestor;
//       // Set toplevel window topmost!
//       SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0,
//           SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
//     }
// #endif
}


void
PreludeWindow::set_progress(int value, int max_value)
{
  progress_value = value;
  progress_max_value = max_value;
  refresh();
}


void
PreludeWindow::set_progress_text(IApp::PreludeProgressText text)
{
  switch (text)
    {
    case IApp::PROGRESS_TEXT_BREAK_IN:
      progress_text = _("Break in %s");
      break;

    case IApp::PROGRESS_TEXT_DISAPPEARS_IN:
      progress_text = _("Disappears in %s");
      break;

    case IApp::PROGRESS_TEXT_SILENT_IN:
      progress_text = _("Silent in %s");
      break;
    }
}


void
PreludeWindow::set_stage(IApp::PreludeStage stage)
{
  const char *icon = NULL;
  switch(stage)
    {
    case IApp::STAGE_INITIAL:
      frame->set_frame_flashing(0);
      frame->set_frame_visible(false);
      icon = "prelude-hint.png";
      break;

    case IApp::STAGE_WARN:
      frame->set_frame_visible(true);
      frame->set_frame_flashing(500);
      frame->set_frame_color(QColor("orange"));
      icon = "prelude-hint-sad.png";
      break;

    case IApp::STAGE_ALERT:
      frame->set_frame_flashing(500);
      frame->set_frame_color(QColor("red"));
      icon = "prelude-hint-sad.png";
      break;

    case IApp::STAGE_MOVE_OUT:
      if (! did_avoid)
        {
          QDesktopWidget *dw = QApplication::desktop();
          const QRect	rect = dw->screenGeometry(screen);

          move(x(), rect.y() + SCREEN_MARGIN);
        }
      break;
    }

  if (icon != NULL)
    {
      std::string file = AssetPath::complete_directory(icon, AssetPath::SEARCH_PATH_IMAGES);
      image->setPixmap(QPixmap(file.c_str()));
    }
}

void
PreludeWindow::on_frame_flash(bool frame_visible)
{
  TRACE_ENTER("PreludeWindow::on_frame_flash");
  flash_visible = frame_visible;
  refresh();
  TRACE_EXIT();
}


bool
PreludeWindow::event(QEvent *event)
{
  TRACE_ENTER_MSG("PreludeWindow::event", event->type());

  if (event->type() == QEvent::HoverEnter)
    {
      QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
      avoid_pointer(hoverEvent->pos().x(), hoverEvent->pos().y());
    }
  bool res = QWidget::event(event);
  
  TRACE_MSG(QApplication::activeModalWidget() << " " << QApplication::activeModalWidget());
  TRACE_EXIT();
  return res;
}


//! Move window if pointer is neat specified location.
void
PreludeWindow::avoid_pointer(int px, int py)
{
  const QRect &geo = geometry();

  px += geo.x();
  py += geo.y();

  QDesktopWidget *dw = QApplication::desktop();
  const QRect	rect = dw->screenGeometry(screen);

  int screen_height = rect.height();
  int top_y = rect.y() + SCREEN_MARGIN;
  int bottom_y = rect.y() + screen_height - geo.height() - SCREEN_MARGIN;
  int winy = geo.y();
  int winx = geo.x();

#ifdef PLATFORM_OS_OSX
  if (!geo.contains(px, py))
    {
      return;
    }
#endif
  std::cout << "avoid_pointer " << px << " " <<py <<std::endl;

  if (winy < top_y + SCREEN_MARGIN)
    {
      winy = bottom_y;
    }
  else if (winy > bottom_y - SCREEN_MARGIN)
    {
      winy = top_y;
    }
  else
    {
      if (py > winy + geo.height()/2)
        {
          winy = top_y;
        }
      else
        {
          winy = bottom_y;
        }
    }

  move(winx, winy);
  did_avoid = true;
}

