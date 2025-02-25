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

#ifndef WORKRAVE_UI_PREFWIDGETS_VALUE_HH
#define WORKRAVE_UI_PREFWIDGETS_VALUE_HH

#include <functional>
#include <vector>

#include "Widget.hh"

namespace ui::prefwidgets
{
  enum class ValueKind
  {
    Spin,
    Slider
  };

  class Value : public WidgetBase<Value, int>
  {
  public:
    Value() = default;
    explicit Value(const std::string &label, int min, int max, ValueKind kind = ValueKind::Spin);
    ~Value() override = default;

    static std::shared_ptr<Value> create(const std::string &label, int min, int max, ValueKind kind = ValueKind::Spin);

    int get_min() const;
    int get_max() const;
    ValueKind get_kind() const;

  private:
    int min{0};
    int max{1};
    ValueKind kind{ValueKind::Spin};
  };
} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_VALUE_HH
