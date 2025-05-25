// Copyright (C) 2025 Rob Caelers & Raymond Penners
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

#ifndef WORKRAVE_UI_DBUSPRELUDEWINDOW_HH
#define WORKRAVE_UI_DBUSPRELUDEWINDOW_HH

#include "ui/IPreludeWindow.hh"
#include "core/IApp.hh"
#include "dbus/IDBus.hh"

class DBusPreludeWindow : public IPreludeWindow
{
public:
  DBusPreludeWindow();
  ~DBusPreludeWindow() override;

  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

  static bool is_gnome_shell_applet_available(workrave::dbus::IDBus::Ptr dbus);

private:
  std::string stage_to_string(workrave::IApp::PreludeStage stage);
  std::string progress_text_to_string(workrave::IApp::PreludeProgressText text);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

#endif // WORKRAVE_UI_DBUSPRELUDEWINDOW_HH
