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
#include "Menus.hh"

#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "ExercisesDialog.hh"

#include "StatusIcon.hh"
#include "ToolkitMenu.hh"

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
  virtual void init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme);

  //!
  virtual void terminate();

  //!
  virtual void run();

  //!
  virtual void grab();

  //!
  virtual void ungrab();

  //!
  virtual std::string get_display_name();

  //!
  virtual IBreakWindow::Ptr create_break_window(int screen, workrave::BreakId break_id, BreakFlags break_flags);

  //!
  virtual IPreludeWindow::Ptr create_prelude_window(int screen, workrave::BreakId break_id);

  //!
  virtual int get_screen_count() const;

  //!
  virtual void show_window(WindowType type);

  //!
  virtual void hide_window(WindowType type);

  //!
  virtual IStatusIcon::Ptr get_status_icon() const;
                                                  
  //!
  virtual void create_oneshot_timer(int ms, boost::function<void ()> func);

public slots:
  void on_timer();
  void on_exercises_closed();
  void on_preferences_closed();

private:
  boost::shared_ptr<QTimer> heartbeat_timer;

  boost::shared_ptr<MainWindow> main_window;
  boost::shared_ptr<PreferencesDialog> preferences_dialog;
  boost::shared_ptr<ExercisesDialog> exercises_dialog;
  boost::shared_ptr<IStatusIcon> status_icon;

  MenuModel::Ptr menu_model;
  ToolkitMenu::Ptr dock_menu;
  SoundTheme::Ptr sound_theme;

  //! Timer signal.
  boost::signals2::signal<void()> timer_signal;

};

class OneshotTimer : public QObject
{
  Q_OBJECT

public:
  OneshotTimer(int ms, boost::function<void ()> func) : func(func)
  {
    QTimer::singleShot(ms, this, SLOT(exec()));
  };

public slots:
  void exec()
  {
    func();
    this->deleteLater();
  };
  
private:
    boost::function<void ()> func;
};

#endif // TOOLKIT_HH
