// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#ifndef WORKRAVE_TASKBAR_PLACEMENT_HH
#define WORKRAVE_TASKBAR_PLACEMENT_HH

#include <algorithm>
#include <optional>
#include <vector>

namespace workrave::taskbar
{
  struct Interval
  {
    int left;
    int right;
  };

  // Coordinates are physical screen pixels. Prefer the rightmost available
  // gap, normally immediately before the notification area. Never move or
  // resize Explorer's controls to make room.
  inline std::optional<int> find_slot(int left, int right, int width, int gap, std::vector<Interval> occupied)
  {
    if (width <= 0 || gap < 0 || right - left < width + (2 * gap))
      {
        return {};
      }
    std::ranges::sort(occupied, [](auto a, auto b) { return a.left < b.left; });
    int cursor = left + gap;
    std::optional<int> result;
    for (auto block: occupied)
      {
        if (block.right <= left || block.left >= right || block.left >= block.right)
          {
            continue;
          }
        int end = std::min(right - gap, block.left - gap);
        if (end - cursor >= width)
          {
            result = end - width;
          }
        cursor = std::max(cursor, block.right + gap);
      }
    if (right - gap - cursor >= width)
      {
        result = right - gap - width;
      }
    return result;
  }
} // namespace workrave::taskbar

#endif
