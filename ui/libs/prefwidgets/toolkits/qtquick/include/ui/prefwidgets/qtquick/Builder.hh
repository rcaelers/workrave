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

#ifndef UI_PREFWIDGETS_QTQUICK_BUILDER_HH
#define UI_PREFWIDGETS_QTQUICK_BUILDER_HH

#include "ui/prefwidgets/Def.hh"
#include "Models.hh"

namespace ui::prefwidgets::qtquick
{
  class Builder
  {
  public:
    // Call before loading standalone QML controls. build() also initializes
    // the module automatically.
    static void initialize();
    static PageModel *build(const std::shared_ptr<Def> &def, QObject *parent = nullptr);
    static void append(const std::shared_ptr<Def> &def, PageModel *page);
  };
} // namespace ui::prefwidgets::qtquick

#endif
