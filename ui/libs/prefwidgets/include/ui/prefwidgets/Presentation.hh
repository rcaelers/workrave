// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

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
