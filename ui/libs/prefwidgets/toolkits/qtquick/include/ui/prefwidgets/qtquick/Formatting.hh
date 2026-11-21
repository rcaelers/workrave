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

#ifndef UI_PREFWIDGETS_QTQUICK_FORMATTING_HH
#define UI_PREFWIDGETS_QTQUICK_FORMATTING_HH

#include <QString>

namespace ui::prefwidgets::qtquick::Formatting
{
  struct Range
  {
    int minimum{0};
    int maximum{0};
    int step{1};
  };

  QString formatTime(int seconds);
  double normalize(int value, Range range);
  int denormalize(double position, Range range);
  int clampStep(int value, int delta, Range range);
  int snapStep(int value, Range range);
} // namespace ui::prefwidgets::qtquick::Formatting

#endif
