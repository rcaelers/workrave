// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef TOOLKIT_WINDOWS_HH
#define TOOLKIT_WINDOWS_HH

#include <memory>
#include <boost/signals2.hpp>

#if defined(HAVE_UNFOLD_AUTO_UPDATE)
#  include "AutoUpdater.hh"
#endif
#include "ui/windows/IToolkitWindows.hh"
#include "Toolkit.hh"

#include "ui/windows/WindowsLocker.hh"
#if defined(HAVE_HARPOON)
#  include "ui/windows/WindowsHarpoonLocker.hh"
#endif

class ToolkitWindows
  : public Toolkit
  , public IToolkitWindows
{
public:
  ToolkitWindows(int argc, char **argv);
  ~ToolkitWindows() override;

  void init(std::shared_ptr<IApplicationContext> app) override;
  void release() override;

  std::shared_ptr<Locker> get_locker() override;

  boost::signals2::signal<bool(MSG *msg), IToolkitWindows::event_combiner> &hook_event() override;
  HWND get_event_hwnd() const override;

private:
  void init_filter();
  void init_gui();
  void init_updater();
  void theme_fixup();
  bool filter_func(MSG *msg);
  static GdkFilterReturn static_filter_func(void *xevent, GdkEvent *event, gpointer data);

private:
  boost::signals2::signal<bool(MSG *msg), IToolkitWindows::event_combiner> event_hook;

#if defined(HAVE_HARPOON)
  std::shared_ptr<WindowsHarpoonLocker> locker;
#else
  std::shared_ptr<WindowsLocker> locker;
#endif
#if defined(HAVE_UNFOLD_AUTO_UPDATE)
  std::shared_ptr<AutoUpdater> updater;
#endif
};

#endif // TOOLKIT_WINDOWS_HH
