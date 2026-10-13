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

#ifndef WORKRAVE_UI_PREFWIDGETS_ENTRY_HH
#define WORKRAVE_UI_PREFWIDGETS_ENTRY_HH

#include <functional>
#include "Widget.hh"

namespace ui::prefwidgets
{
  class Entry : public WidgetBase<Entry, std::string>
  {
  public:
    Entry() = default;
    explicit Entry(const std::string &label);
    ~Entry() override = default;

    static std::shared_ptr<Entry> create();
    static std::shared_ptr<Entry> create(const std::string &label);

    std::shared_ptr<Entry> assign(const std::string &txt);
    std::shared_ptr<Entry> assign(std::function<std::string()> func);
    std::string get_content() const;

  private:
    std::string content;
    std::function<std::string()> fill_func;
  };
} // namespace ui::prefwidgets

#endif // WORKRAVE_UI_PREFWIDGETS_ENTRY_HH
