// Copyright (C) 20014 Rob Caelers <robc@krandor.org>
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

#ifndef MOUSEMONITOR_HH
#define MOUSEMONITOR_HH

#include <memory>
#include <functional>

class MouseMonitor
{
public:
  using Ptr = std::shared_ptr<MouseMonitor>;

  explicit MouseMonitor(std::function<void(int, int)> func);
  ~MouseMonitor();

  void start();
  void stop();

private:
  class Private;
  std::unique_ptr<Private> priv;
};

#endif // MOUSEMONITOR_HH
