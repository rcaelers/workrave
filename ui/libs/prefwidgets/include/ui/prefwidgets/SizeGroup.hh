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

#ifndef WORKRAVE_UI_PREFWIDGETS_SIZEGROUP_HH
#define WORKRAVE_UI_PREFWIDGETS_SIZEGROUP_HH

#include "Widget.hh"

namespace ui::prefwidgets
{
  class SizeGroup
  {
  public:
    SizeGroup() = default;
    explicit SizeGroup(Orientation orientation = Orientation::Horizontal);
    ~SizeGroup() = default;

    static std::shared_ptr<SizeGroup> create(Orientation orientation = Orientation::Horizontal);

    Orientation get_orientation() const;

  private:
    Orientation orientation{Orientation::Horizontal};
  };
} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_SIZEGROUP_HH
