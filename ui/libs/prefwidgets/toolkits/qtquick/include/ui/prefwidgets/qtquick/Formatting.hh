// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers
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
