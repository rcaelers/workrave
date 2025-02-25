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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Builder.hh"

#include "ChoiceWidget.hh"
#include "ToggleWidget.hh"
#include "FrameWidget.hh"
#include "BoxWidget.hh"
#include "SpinWidget.hh"
#include "SliderWidget.hh"
#include "TimeEntryWidget.hh"
#include "SizeGroup.hh"

using namespace ui::prefwidgets::qt;

void
Builder::handle_size_groups(std::shared_ptr<ui::prefwidgets::Widget> def)
{
  for (auto sg: def->get_size_groups())
    {
      if (size_groups.find(sg) == size_groups.end())
        {
          auto size_group = std::make_shared<SizeGroup>(sg);
          size_groups[sg] = size_group;
        }
    }
}

std::shared_ptr<SizeGroup>
Builder::get_size_group(std::shared_ptr<ui::prefwidgets::SizeGroup> def)
{
  if (size_groups.find(def) != size_groups.end())
    {
      return size_groups[def];
    }
  return {};
}

void
Builder::build(std::shared_ptr<ui::prefwidgets::Widget> def, std::shared_ptr<ContainerWidget> container)
{
  std::shared_ptr<Widget> widget;
  handle_size_groups(def);

  if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Choice>(def); d)
    {
      widget = std::make_shared<ChoiceWidget>(d, container, this);
    }
  if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Value>(def); d)
    {
      if (d->get_kind() == ValueKind::Spin)
        {
          widget = std::make_shared<SpinWidget>(d, container, this);
        }
      if (d->get_kind() == ValueKind::Slider)
        {
          widget = std::make_shared<SliderWidget>(d, container, this);
        }
    }
  if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Toggle>(def); d)
    {
      widget = std::make_shared<ToggleWidget>(d, container, this);
    }
  if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Time>(def); d)
    {
      widget = std::make_shared<TimeEntryWidget>(d, container, this);
    }
  if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Frame>(def); d)
    {
      auto container_widget = std::make_shared<FrameWidget>(d, container, this);
      for (auto &child_def: d->get_content())
        {
          build(child_def, container_widget);
        }
      widget = container_widget;
    }
  if (auto d = std::dynamic_pointer_cast<ui::prefwidgets::Box>(def); d)
    {
      auto container_widget = std::make_shared<BoxWidget>(d, container, this);
      for (auto &child_def: d->get_content())
        {
          build(child_def, container_widget);
        }
      widget = container_widget;
    }

  if (widget)
    {
      container->add(widget);
    }
}
