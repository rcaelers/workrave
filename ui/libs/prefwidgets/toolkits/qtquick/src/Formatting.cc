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

#include "ui/prefwidgets/qtquick/Formatting.hh"
#include <algorithm>
#include <cmath>

namespace ui::prefwidgets::qtquick::Formatting
{
  QString formatTime(int seconds)
  {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int remainder = seconds % 60;
    if (hours > 0)
      {
        return QStringLiteral("%1:%2:%3")
          .arg(hours)
          .arg(minutes, 2, 10, QLatin1Char('0'))
          .arg(remainder, 2, 10, QLatin1Char('0'));
      }
    return QStringLiteral("%1:%2").arg(minutes).arg(remainder, 2, 10, QLatin1Char('0'));
  }

  double normalize(int value, Range range)
  {
    if (range.maximum <= range.minimum)
      {
        return 0.0;
      }
    return std::clamp(static_cast<double>(value - range.minimum) / (range.maximum - range.minimum), 0.0, 1.0);
  }

  int denormalize(double position, Range range)
  {
    if (range.maximum <= range.minimum || range.step <= 0)
      {
        return range.minimum;
      }
    double raw = range.minimum + (position * (range.maximum - range.minimum));
    int snapped = static_cast<int>(std::round(raw / range.step)) * range.step;
    return std::clamp(snapped, range.minimum, range.maximum);
  }

  int clampStep(int value, int delta, Range range)
  {
    if (range.maximum < range.minimum || range.step <= 0)
      {
        return range.minimum;
      }
    return std::clamp(value + (delta * range.step), range.minimum, range.maximum);
  }

  int snapStep(int value, Range range)
  {
    if (range.maximum < range.minimum || range.step <= 0)
      {
        return range.minimum;
      }
    int snapped = static_cast<int>(std::round(static_cast<double>(value) / range.step)) * range.step;
    return std::clamp(snapped, range.minimum, range.maximum);
  }
} // namespace ui::prefwidgets::qtquick::Formatting
