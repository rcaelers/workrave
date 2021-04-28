// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#ifndef TOOLKITPLATFORMWINDOWS_HH
#define TOOLKITPLATFORMWINDOWS_HH

#include <memory>

#include "IToolkitPlatform.hh"

class ToolkitPlatformWindows : public IToolkitPlatform
{
public:
  ToolkitPlatformWindows();
  ~ToolkitPlatformWindows() override;

  QPixmap get_desktop_image() override;

  void foreground() override;
  void restore_foreground() override;

  void lock() override;
  void unlock() override;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> pimpl;
};

#endif // TOOLKITPLATFORMWINDOWS_HH