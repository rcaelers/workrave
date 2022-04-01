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

#ifndef WORKRAVE_UI_PREFWIDGETS_SECTION_HH
#define WORKRAVE_UI_PREFWIDGETS_SECTION_HH

#include "Widget.hh"

namespace ui::prefwidgets
{
  class Section : public ContainerBase<Section>
  {
  public:
    Section() = default;
    explicit Section(const std::string &name);
    ~Section() override = default;

    static std::shared_ptr<Section> create(const std::string &name);

    std::string get_name() const;

  private:
    std::string name;
  };
} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_SECTION_HH
