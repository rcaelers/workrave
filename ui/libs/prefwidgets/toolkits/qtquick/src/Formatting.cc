// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers
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
