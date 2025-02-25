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

#ifndef WORKRAVE_UI_APPHOLD_HH
#define WORKRAVE_UI_APPHOLD_HH

#include "ui/IToolkit.hh"
#include <memory>

class AppHold
{
public:
  using Ptr = std::shared_ptr<AppHold>;

  explicit AppHold(std::shared_ptr<IToolkit> toolkit);
  ~AppHold();

  void set_hold(bool h);
  void hold();
  void release();

private:
  std::weak_ptr<IToolkit> toolkit;
  bool held{false};
};

#endif // WORKRAVE_UI_APPHOLD_HH
