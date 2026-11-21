// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef APPLICATION_MACOS_HH
#define APPLICATION_MACOS_HH

#include <memory>

#include "Application.hh"

#include "ui/macos/MacOSSleepMonitor.hh"
#include "ui/macos/MacOSDock.hh"

class ApplicationMacOS : public Application
{
public:
  ApplicationMacOS(int argc, char **argv, std::shared_ptr<IToolkitFactory> toolkit_factory);
  ~ApplicationMacOS() override = default;

  void init_platform_pre() override;
  void init_platform_post() override;

private:
  std::shared_ptr<MacOSDock> dock;
  std::shared_ptr<MacOSSleepMonitor> sleep_monitor;
  std::unique_ptr<MacOSAppNapDisabler> app_nap_disabler;
};

#endif // APPLICATION_MACOS_HH
