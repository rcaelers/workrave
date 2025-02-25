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

#ifndef WORKRAVE_UI_PREFWIDGETS_DEF_HH
#define WORKRAVE_UI_PREFWIDGETS_DEF_HH

#include <functional>
#include <memory>

#include "Widget.hh"

namespace ui::prefwidgets
{
  class Def : public std::enable_shared_from_this<Def>
  {
  public:
    Def() = default;
    explicit Def(const std::string &page, const std::string &panel)
      : page(page)
      , panel(panel)
    {
    }
    virtual ~Def() = default;

    virtual std::string get_id()
    {
      return page + "." + panel + ".";
    }

    std::string get_page() const
    {
      return page;
    }

    std::string get_panel() const
    {
      return panel;
    }

    std::shared_ptr<Widget> get_widget() const
    {
      return widget;
    }

    auto operator<<(std::shared_ptr<Widget> w)
    {
      widget = w;
      return widget;
    }

  private:
    std::string page;
    std::string panel;
    std::shared_ptr<Widget> widget;
  };

  class PanelDef : public Def
  {
  public:
    PanelDef() = default;
    PanelDef(const std::string &page, const std::string &panel, const std::string &label)
      : Def(page, panel)
      , label(label)
    {
    }
    ~PanelDef() override = default;

    static std::shared_ptr<PanelDef> create(const std::string &page, const std::string &panel, const std::string &label)
    {
      return std::make_shared<PanelDef>(page, panel, label);
    }

    std::string get_label() const
    {
      return label;
    }

  private:
    std::string label;
  };

  template<typename S, typename T>
  std::shared_ptr<S> operator<<(std::shared_ptr<S> s, std::shared_ptr<T> t)
  requires std::is_base_of_v<Def, S> && std::is_base_of_v<Widget, T>
  {
    *s << t;
    return s;
  }

} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_DEF_HH
