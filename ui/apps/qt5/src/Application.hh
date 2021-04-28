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

#ifndef APPLICATION_HH
#define APPLICATION_HH

#include <list>
#include <vector>
#include <memory>

#include <QCoreApplication>

#include "commonui/Session.hh"
#include "commonui/SoundTheme.hh"
#include "config/Config.hh"
#include "core/IApp.hh"
#include "updater/Updater.hh"
#include "utils/ScopedConnections.hh"

#include "IApplication.hh"
#include "IBreakWindow.hh"
#include "IPreludeWindow.hh"
#include "IToolkit.hh"
#include "Menus.hh"

class Application
  : public IApplication
  , public workrave::IApp
{
public:
  typedef std::shared_ptr<Application> Ptr;

  Application(int argc, char **argv, std::shared_ptr<IToolkit> toolkit);
  ~Application() override;

  SoundTheme::Ptr get_sound_theme() const;
  void main();

  // IApp methods
  void create_prelude_window(workrave::BreakId break_id) override;
  void create_break_window(workrave::BreakId break_id, workrave::utils::Flags<workrave::BreakHint> break_hint) override;
  void hide_break_window() override;
  void show_break_window() override;
  void refresh_break_window() override;
  void set_break_progress(int value, int max_value) override;
  void set_prelude_stage(PreludeStage stage) override;
  void set_prelude_progress_text(PreludeProgressText text) override;

  // IApplication
  void restbreak_now() override;
  void terminate() override;

private:
  bool on_timer();
  void init_platform();
  void init_core();
  void init_sound_player();
  void init_bus();
  void init_session();
  void init_startup_warnings();
  void init_updater();

  void on_status_icon_balloon_activate(const std::string &id);
  void on_status_icon_activate();

  void on_break_event(workrave::BreakId break_id, workrave::BreakEvent event);
  void on_operation_mode_warning_timer();

private:
  typedef std::vector<IBreakWindow::Ptr> BreakWindows;
  typedef BreakWindows::iterator BreakWindowsIter;

  typedef std::vector<IPreludeWindow::Ptr> PreludeWindows;
  typedef PreludeWindows::iterator PreludeWindowsIter;

  std::shared_ptr<IToolkit> toolkit;
  std::shared_ptr<workrave::ICore> core;
  std::shared_ptr<Menus> menus;
  std::shared_ptr<workrave::updater::Updater> updater;
  int argc{0};
  char **argv{nullptr};
  SoundTheme::Ptr sound_theme;
  BreakWindows break_windows;
  PreludeWindows prelude_windows;
  workrave::BreakId active_break_id{workrave::BREAK_ID_NONE};
  Session::Ptr session;
  bool muted{false};

  scoped_connections connections;
};

inline SoundTheme::Ptr
Application::get_sound_theme() const
{
  return sound_theme;
}
#endif // APPLICATION_HH
