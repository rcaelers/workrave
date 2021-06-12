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

#ifndef DOCK_HH
#define DOCK_HH

#include <QObject>

#include <memory>

#include "utils/ScopedConnections.hh"

#include "core/ICore.hh"

class DockPrivate;

class Dock : public QObject
{
  Q_OBJECT

public:
  Dock();

private:
  void on_operation_mode_changed(workrave::OperationMode m);

private:
  std::shared_ptr<DockPrivate> priv;
};

#endif // DOCK_HH
