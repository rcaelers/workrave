// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_QT_DBUSPRELUDEWINDOW_HH
#define WORKRAVE_UI_QT_DBUSPRELUDEWINDOW_HH

#include <memory>
#include <string>

#include "ui/Prelude.hh"
#include "ui/IPreludeWindow.hh"
#include "core/IApp.hh"

//! Prelude rendered by the Workrave GNOME Shell extension over D-Bus.
/*!
 *  On GNOME the compositor will not let a client position its own windows, so
 *  the shell extension draws the prelude instead. This class only drives it;
 *  it owns no window of its own.
 *
 *  Qt counterpart of the gtkmm DBusPreludeWindow, using QtDBus rather than gio.
 */
class DBusPreludeWindow
  : public IPreludeWindow
  , Prelude
{
public:
  explicit DBusPreludeWindow(workrave::BreakId break_id);
  ~DBusPreludeWindow() override;

  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

  static bool is_gnome_shell_applet_available();

private:
  class Impl;
  std::unique_ptr<Impl> impl;
  workrave::BreakId break_id;
  std::string progress_text;
};

#endif // WORKRAVE_UI_QT_DBUSPRELUDEWINDOW_HH
