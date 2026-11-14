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

#include "TaskbarPlacement.hh"

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <exception>
#include <random>

using workrave::taskbar::find_slot;
using workrave::taskbar::Interval;

namespace
{
  void require(bool value)
  {
    if (!value)
      {
        std::cerr << "Taskbar placement regression\n";
        std::exit(EXIT_FAILURE);
      }
  }

  // Independent pixel-by-pixel oracle: no sorting or interval merging.
  std::optional<int> brute_force(int left, int right, int width, int gap, const std::vector<Interval> &occupied)
  {
    if (width <= 0 || gap < 0)
      {
        return {};
      }
    for (int x = right - gap - width; x >= left + gap; --x)
      {
        bool fits = true;
        for (auto block: occupied)
          {
            if (block.left < block.right && block.right > left && block.left < right && x < block.right + gap
                && x + width > block.left - gap)
              {
                fits = false;
              }
          }
        if (fits)
          {
            return x;
          }
      }
    return {};
  }
} // namespace

int
main()
try
  {
    require(find_slot(0, 1920, 100, 6, {{.left = 0, .right = 600}, {.left = 1700, .right = 1920}}) == 1594);
    require(!find_slot(0, 1920, 100, 6, {{.left = 0, .right = 1700}, {.left = 1700, .right = 1920}}));
    require(find_slot(-1920, 0, 200, 12, {{.left = -300, .right = 0}, {.left = -1800, .right = -1200}}) == -512);
    require(find_slot(
              0,
              1000,
              100,
              6,
              {{.left = 900, .right = 1000}, {.left = 0, .right = 200}, {.left = 100, .right = 500}, {.left = 200, .right = 300}})
            == 794);
    require(!find_slot(0, 100, 100, 6, {}));
    require(!find_slot(0, 1000, 0, 6, {}));
    require(find_slot(0, 112, 100, 6, {}) == 6);

    // NOLINTNEXTLINE(bugprone-random-generator-seed) -- Reproducible regression layouts.
    std::mt19937 random(0xdecbaad);
    for (int trial = 0; trial < 20000; ++trial)
      {
        int left = static_cast<int>(random() % 400) - 200;
        int right = left + static_cast<int>(random() % 600);
        int width = static_cast<int>(random() % 160);
        int gap = static_cast<int>(random() % 20);
        std::vector<Interval> occupied;
        int count = static_cast<int>(random() % 20);
        for (int i = 0; i < count; ++i)
          {
            int x = left - 100 + static_cast<int>(random() % 800);
            occupied.push_back({.left = x, .right = x + static_cast<int>(random() % 240) - 20});
          }
        require(find_slot(left, right, width, gap, occupied) == brute_force(left, right, width, gap, occupied));
      }
    std::cout << "Taskbar placement: fixtures and 20000 randomized layouts passed\n";
  }
catch (const std::exception &error)
  {
    std::fputs("Regression test failed: ", stderr);
    std::fputs(error.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  }
