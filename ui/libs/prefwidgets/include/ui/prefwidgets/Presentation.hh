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

#ifndef UI_PREFWIDGETS_PRESENTATION_HH
#define UI_PREFWIDGETS_PRESENTATION_HH

#include <string>
#include <vector>

namespace ui::prefwidgets
{
  // Common, optional presentation text and colors. Toolkit backends may fall
  // back to ordinary controls without changing values or configuration bindings.
  struct Presentation
  {
    struct Color
    {
      // Optional host theme role; explicit colors are fallbacks.
      std::string role;
      std::string light;
      std::string dark;
    };

    std::string name;
    std::string label;
    std::string hint;
    Color accent;
  };

  struct ChoicePresentation
  {
    enum class Control
    {
      Default,
      Cards,
      Segmented,
    };

    // A named asset supplied by the host's renderer. The definition contains
    // no drawing geometry, resource URLs or toolkit components.
    struct Preview
    {
      std::string asset;
      std::string description;
    };

    Control control{Control::Default};
    std::string name;
    std::string label;
    std::string hint;
    Presentation::Color accent;
    std::vector<std::string> options;
    std::vector<Preview> previews;
    int preview_height{52};
  };

  struct ValuePresentation
  {
    enum class Control
    {
      Default,
      Duration,
    };

    Control control{Control::Default};
    std::string name;
    std::string label;
    std::string hint;
    Presentation::Color accent;
    int slider_min{0};
    int slider_max{0};
    std::vector<int> ticks;
  };
} // namespace ui::prefwidgets

#endif
