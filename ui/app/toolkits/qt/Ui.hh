// Copyright (C) 2014, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef UI_HH
#define UI_HH

#include <QCoreApplication>
#include <QString>

#include "core/CoreTypes.hh"
#include "ui/UiTypes.hh"
#include "ui/SoundTheme.hh"

class Ui
{
  Q_DECLARE_TR_FUNCTIONS(Application);

public:
  static auto get_break_name(workrave::BreakId id) -> QString;
  static auto get_break_icon_filename(workrave::BreakId id) -> QString;
  static auto get_status_icon_filename(OperationModeIcon icon) -> QString;
  static auto get_sound_event_name(SoundEvent event) -> QString;
};

#endif // UI_HH
