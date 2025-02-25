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

#ifndef WORKRAVE_UI_PREFWIDGETS_QT_BUILDER_HH
#define WORKRAVE_UI_PREFWIDGETS_QT_BUILDER_HH

#include <string>
#include <map>
#include <memory>

#include "ui/prefwidgets/Widgets.hh"

#include "ContainerWidget.hh"
#include "SizeGroup.hh"
#include "BuilderRegistry.hh"

namespace ui::prefwidgets::qt
{
  class Builder : public BuilderRegistry
  {
  public:
    explicit Builder() = default;
    ~Builder() override = default;

    void build(std::shared_ptr<ui::prefwidgets::Widget> def, std::shared_ptr<ContainerWidget> container);
    std::shared_ptr<SizeGroup> get_size_group(std::shared_ptr<ui::prefwidgets::SizeGroup> def) override;

  private:
    void handle_size_groups(std::shared_ptr<ui::prefwidgets::Widget> def);

  private:
    std::map<std::shared_ptr<ui::prefwidgets::SizeGroup>, std::shared_ptr<SizeGroup>> size_groups;
  };
} // namespace ui::prefwidgets::qt

#endif // WORKRAVE_UI_PREFWIDGETS_QT_BUILDER_HH
