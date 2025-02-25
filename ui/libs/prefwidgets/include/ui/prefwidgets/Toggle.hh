// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_PREFWIDGETS_TOGGLE_HH
#define WORKRAVE_UI_PREFWIDGETS_TOGGLE_HH

#include <string>
#include <functional>

#include "Widget.hh"

namespace ui::prefwidgets
{
  class Toggle : public WidgetBase<Toggle, bool>
  {
  public:
    Toggle() = default;
    explicit Toggle(const std::string &label);
    ~Toggle() override = default;

    static std::shared_ptr<Toggle> create();
    static std::shared_ptr<Toggle> create(const std::string &label);
  };
} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_TOGGLE_HH
