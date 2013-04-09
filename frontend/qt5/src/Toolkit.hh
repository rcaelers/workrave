// Tookit.hh
//
// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef TOOLKIT_HH
#define TOOLKIT_HH

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <QApplication>  
#include <QTimer>

#include "IToolkit.hh"
#include "CoreTypes.hh"

#include "MainWindow.hh"

class Toolkit : public QApplication, public IToolkit
{
  Q_OBJECT

public:
  
  typedef boost::shared_ptr<Toolkit> Ptr;

  static IToolkit::Ptr create(int argc, char **argv);

  Toolkit(int argc, char **argv);
  virtual ~Toolkit();

  virtual boost::signals2::signal<void()> &signal_timer();
  
  //!
  virtual void init();
  
  //! 
  virtual void run();

  //! 
  virtual void grab();

  //!
  virtual void ungrab();

  //!
  virtual std::string get_display_name();

  //!
  virtual IBreakWindow::Ptr create_break_window(HeadInfo &head, workrave::BreakId break_id, IBreakWindow::BreakFlags break_flags);

  //!
  virtual IPreludeWindow::Ptr create_prelude_window(HeadInfo &head, workrave::BreakId break_id);

public slots:
  void on_timer();
  
private:
  boost::shared_ptr<QTimer> heartbeat_timer;
  boost::shared_ptr<MainWindow> main_window;

  //! Timer signal.
  boost::signals2::signal<void()> timer_signal;
  
};

#endif // TOOLKIT_HH
